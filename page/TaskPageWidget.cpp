#include "TaskPageWidget.h"
#include "ui_TaskPageWidget.h"
#include "PreviewDialog.h"
#include "page/StartupPageWidget.h"
#include "MainWindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QMovie>
#include <QVBoxLayout>
#include <QGraphicsSceneMouseEvent>

#define MAX_DELAY_PHOTOS_NUMBER     2
#define MAX_DELAY_TEMPLATES_NUMBER  2
#define MAX_DELAY_ALBUMS_NUMBER     2

QDialog *TaskPageWidget::m_pLoadingDlg = NULL;

TaskPageWidget::TaskPageWidget(int tabId, const QString &taskFile, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskPageWidget),
    m_tabId(tabId),
    m_taskParser(taskFile, parent),
    m_pPhotosLoader(new LoaderThread(ViewType_Photo)),
    m_pTemplatesLoader(new LoaderThread(ViewType_Template)),
    m_pAlbumsLoader(new LoaderThread(ViewType_Album)),
    m_bCollapsed(false),
    m_bChanged(false),
    m_pFocusScene(NULL),
    m_scensVector(GraphicsScenesVector(PictureGraphicsScene::SceneType_End))
{
    ui->setupUi(this);

    /* Setups graphics scenes */
    m_pPhotosScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Grid,
                                              PictureGraphicsScene::SceneType_Photos,
                                              ui->photosGraphicsView,
                                              this);
    m_scensVector.insert(PictureGraphicsScene::SceneType_Photos, m_pPhotosScene);

    m_pTemplatePage = new TemplatePageWidget(false);
    ui->rightVerticalLayout->addWidget(m_pTemplatePage);

    m_pTemplatesScene = new PictureGraphicsScene(Qt::gray,
                                                 PictureGraphicsScene::LayoutMode_Grid,
                                                 PictureGraphicsScene::SceneType_Templates,
                                                 m_pTemplatePage->getView(),
                                                 this);
    m_scensVector.insert(PictureGraphicsScene::SceneType_Templates, m_pTemplatesScene);

    m_pAlbumsScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Horizontality,
                                              PictureGraphicsScene::SceneType_Albums,
                                              ui->albumsGraphicsView,
                                              this);
    m_scensVector.insert(PictureGraphicsScene::SceneType_Albums, m_pAlbumsScene);

    m_pEditPage = new EditPageWidget(this);
    ui->mainHorizontalLayout->addWidget(m_pEditPage);
    m_pEditPage->hide();
    connect(m_pEditPage, SIGNAL(editEntered(bool)), SLOT(enterEdit(bool)));

    m_pPreviewDlg = new PreviewDialog(this);
    connect(m_pPreviewDlg, SIGNAL(itemDetached(QGraphicsScene*,QString)), SLOT(detachItem(QGraphicsScene*,QString)));

    connect(m_pPhotosLoader, SIGNAL(itemAdded(int,QString,qreal,Qt::Axis,int)),
            SLOT(addItem(int,QString,qreal,Qt::Axis,int)), Qt::BlockingQueuedConnection);
    connect(m_pTemplatesLoader, SIGNAL(itemAdded(int,QString,int)),
            SLOT(addItem(int,QString,int)), Qt::BlockingQueuedConnection);
    connect(m_pAlbumsLoader, SIGNAL(itemAdded(int,QStringList,QString)),
            SLOT(addItem(int,QStringList,QString)), Qt::BlockingQueuedConnection);

    connect(m_pPhotosLoader, SIGNAL(loadFinished(int)), SLOT(finishLoad(int)), Qt::BlockingQueuedConnection);
    connect(m_pTemplatesLoader, SIGNAL(loadFinished(int)), SLOT(finishLoad(int)), Qt::BlockingQueuedConnection);
    connect(m_pAlbumsLoader, SIGNAL(loadFinished(int)), SLOT(finishLoad(int)), Qt::BlockingQueuedConnection);

    if (m_taskParser.isValid())
    {
        QTimer::singleShot(20, this, SLOT(updateViews()));
    }
    else
    {
        saveFile(Task_Save);
    }
}

TaskPageWidget::~TaskPageWidget()
{
    m_pPhotosLoader->disconnect(SIGNAL(itemAdded(int,QString,qreal,Qt::Axis,int)));
    delete m_pPhotosLoader;

    m_pTemplatesLoader->disconnect(SIGNAL(itemAdded(int,QString,int)));
    delete m_pTemplatesLoader;

    m_pAlbumsLoader->disconnect(SIGNAL(itemAdded(int,QStringList,QString)));
    delete m_pAlbumsLoader;

    if (m_pLoadingDlg)
    {
        delete m_pLoadingDlg;
        m_pLoadingDlg = NULL;
    }

    delete ui;
}

inline void TaskPageWidget::noticeChanged()
{
    if (!m_bChanged)
    {
        m_bChanged = true;
        emit changed(m_tabId);
    }

    if (m_pPhotosScene == PictureProxyWidget::getFocusScene())
    {
        int times = 0;
        PictureProxyWidget *proxyWidget = NULL;
        GraphicsItemsList items = m_pPhotosScene->items();

        foreach (QGraphicsItem *item, items)
        {
            if ((proxyWidget = static_cast<PictureProxyWidget *>(item)))
            {
                PhotoChildWidget &childWidget = (PhotoChildWidget &)proxyWidget->getChildWidget();
                if (childWidget.usedTimes())
                {
                    times++;
                }
            }
        }

        QString photosUsage = tr("共导入 %1 张图片").arg(items.size());
        if (times)
        {
            photosUsage += tr("，已使用 %2 张").arg(times);
        }
        ui->photosLabel->setText(photosUsage);
    }

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_pPhotosScene << m_pTemplatesScene << m_pAlbumsScene << PictureProxyWidget::getFocusScene();
}

char *TaskPageWidget::saveFile(uchar mode)
{
    char *pTaskName = NULL;

    if (Task_Save == mode && !m_taskParser.isValid())
    {
        m_taskParser.saveTask();
    }

    if (Task_Saveas == mode)
    {
        QString taskName, destFile, srcFile = m_taskParser.fileName();
        StartupPageWidget startup(parentWidget());

        destFile = srcFile;

        if (srcFile != startup.getTaskFile(mode, destFile, taskName) && !taskName.isEmpty())
        {
            if (!m_bChanged)
            {
                if (m_taskParser.open(QIODevice::ReadOnly))
                {
                    QByteArray data = m_taskParser.readAll();
                    m_taskParser.close();
                    m_taskParser.setParsingFile(destFile);
                    m_taskParser.open(QIODevice::WriteOnly);
                    m_taskParser.write(data);
                    m_taskParser.close();
                }
            }
            else
            {
                m_taskParser.setParsingFile(destFile);
                saveChanges();
            }

            int len = taskName.length() + 8;
            pTaskName = new char[len];
            memset(pTaskName, 0, len);
            strcpy(pTaskName, taskName.toStdString().c_str());
        }
    }

    return pTaskName;
}

void TaskPageWidget::saveChanges()
{
    if (!m_bChanged)
    {
        return;
    }

    QVariantMap conds;
    QVariantList photos, templates, albums;

    conds.insert("search_kind", "children");
    conds.insert("search_style", "classical");
    templates << conds;

    m_pPhotosScene->checkChanges(photos);
    m_pTemplatesScene->checkChanges(templates);
    m_pAlbumsScene->checkChanges(albums);

    if (m_photosList != photos || m_templatesList != templates || m_albumsList != albums)
    {
        if (m_photosList != photos)
        {
            m_photosList = photos;
        }

        if (m_templatesList != templates)
        {
            m_templatesList = templates;
        }

        if (m_albumsList != albums)
        {
            m_albumsList = albums;
        }

        m_taskParser.saveTask(m_photosList, m_templatesList, m_albumsList);
        m_bChanged = false;
    }
}

void TaskPageWidget::updateViews()
{
    if (!m_taskParser.openTask(m_photosList, m_templatesList, m_albumsList))
    {
        return;
    }

    //qDebug() << __FILE__ << __LINE__ << m_templatesList;

    if (MAX_DELAY_PHOTOS_NUMBER > m_photosList.size())
    {
        loadViewItems(m_photosList, ViewType_Photo);
    }
    else
    {
        m_pPhotosLoader->loadList(m_photosList);
        m_pPhotosLoader->begin();
    }

    if (MAX_DELAY_TEMPLATES_NUMBER > m_templatesList.size())
    {
        loadViewItems(m_templatesList, ViewType_Template);
    }
    else
    {
        m_pTemplatesLoader->loadList(m_templatesList);
        m_pTemplatesLoader->begin();
    }

    if (MAX_DELAY_ALBUMS_NUMBER > m_albumsList.size())
    {
        loadViewItems(m_albumsList, ViewType_Album);
    }
    else
    {
        m_pAlbumsLoader->loadList(m_albumsList);
        m_pAlbumsLoader->begin();
    }

    if (m_pPhotosLoader->isActive() || m_pTemplatesLoader->isActive() || m_pAlbumsLoader->isActive())
    {
        if (!m_pLoadingDlg)
        {
            m_pLoadingDlg = new QDialog(0, Qt::FramelessWindowHint);
            QVBoxLayout *vbl = new QVBoxLayout;

            QLabel *movieLabel = new QLabel(m_pLoadingDlg);
            movieLabel->setFixedSize(126, 22);

            QMovie *movie = new QMovie(":/images/loading.gif");
            movie->setSpeed(100);
            movie->start();
            movieLabel->setMovie(movie);

            QLabel *textLabel = new QLabel(m_pLoadingDlg);
            QFont font = textLabel->font();
            font.setBold(true);
            textLabel->setFixedHeight(16);
            textLabel->setFont(font);
            textLabel->setText("正在加载...");
            textLabel->setStyleSheet("color:blue;");

            vbl->addWidget(movieLabel);
            vbl->addWidget(textLabel);
            vbl->setMargin(0);

            m_pLoadingDlg->setFixedSize(126, 38);
            m_pLoadingDlg->setLayout(vbl);
            m_pLoadingDlg->setAttribute(Qt::WA_TranslucentBackground);
        }

        QPoint pos((width() - m_pLoadingDlg->width()) / 2, (height() - m_pLoadingDlg->height()) / 2);
        QPoint globalPoint(this->mapToGlobal(QPoint(0, 0)));
        m_pLoadingDlg->move(globalPoint + pos);
        m_pLoadingDlg->exec();
    }
}

void TaskPageWidget::addItem(int index, const QString &file, qreal angle, Qt::Axis axis, int usedTimes)
{   
    PhotoChildWidget *childWidget = new PhotoChildWidget(index, file, angle, axis, usedTimes, this);
    m_pPhotosScene->insertProxyWidget(index, new PhotoProxyWidget(childWidget), file);
    m_pPhotosScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Photo:" << index << file << angle << axis << usedTimes;
}

void TaskPageWidget::addItem(int index, const QString &file, int usedTimes)
{
    TemplateChildWidget *childWidget = new TemplateChildWidget(index, file, usedTimes, this);
    m_pTemplatesScene->insertProxyWidget(index, new TemplateProxyWidget(childWidget), file);
    m_pTemplatesScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Template:" << index << file;
}

void TaskPageWidget::addItem(int index, const QStringList &filesList, const QString &file)
{
    AlbumProxyWidget *proxyWidget = new AlbumProxyWidget(new AlbumChildWidget(index, filesList, file, this));
    m_pAlbumsScene->insertProxyWidget(index, proxyWidget);
    m_pAlbumsScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Album:" << index << filesList << file;
}

void TaskPageWidget::finishLoad(int from)
{
    if (from)
    {
        noticeChanged();
    }
    else
    {
        if (m_pLoadingDlg && !m_pPhotosLoader->isActive() && !m_pTemplatesLoader->isActive() && !m_pAlbumsLoader->isActive())
        {
            m_pLoadingDlg->accept();
            m_pPhotosScene->setLoadFromDisk(true);
            m_pTemplatesScene->setLoadFromDisk(true);
        }
    }
}

void TaskPageWidget::loadViewItems(const QVariantList &recordsList, ViewType view)
{
    foreach (QVariant list, recordsList)
    {
        QVariantMap recordsMap = list.toMap();
        int index = recordsMap["index"].toInt();

        if (ViewType_Album != view)
        {
            QString file = ViewType_Photo == view ? recordsMap["picture_file"].toString() : recordsMap["template_file"].toString();
            if (file.isEmpty())
            {
                continue;
            }

            int usedTimes = recordsMap["used_times"].toInt();

            if (ViewType_Photo == view)
            {
                qreal angle = recordsMap["rotation_angle"].toDouble();
                Qt::Axis axis = (Qt::Axis)recordsMap["rotation_axis"].toInt();
                //qDebug() << __FILE__ << __LINE__ << "Photo >" << ":" << index << file << angle;
                addItem(index, file, angle, axis, usedTimes);
            }
            else
            {
                //uchar locations = recordsMap["locations"].toInt();
                //QString kind = records["search_kind"].toString();
                //QString style = records["search_style"].toString();
                //qDebug() << __FILE__ << __LINE__ << "Template >" << ":" << file << locations;
                addItem(index, file, usedTimes);
            }
        }
        else
        {
            QStringList photosList = recordsMap["photos_list"].toStringList();
            QString tmplFile = recordsMap["template_file"].toString();
            //qDebug() << __FILE__ << __LINE__ << "Albums:>" << index << "," << photosList << "," << tmplFile;
            addItem(index, photosList, tmplFile);
        }
    }

    //qDebug("%s(%d): loads %d view items cost %d ms in total!", __FILE__ , __LINE__, view, tm.elapsed());
}

void TaskPageWidget::resizeEvent(QResizeEvent *)
{
    if (m_pEditPage->isHidden())
    {
        m_pPhotosScene->adjustViewLayout();
        m_pTemplatesScene->adjustViewLayout();
    }
}

bool TaskPageWidget::eventFilter(QObject *watched, QEvent *event)
{
    QString focusArea, asking;
    PictureGraphicsScene *focusScene = NULL;
    PictureProxyWidget *proxyWidget = NULL;
    GraphicsItemsList items;

    if (m_pPhotosScene == watched)
    {
        focusScene = m_pPhotosScene;
        focusArea = tr("照片库");
    }
    else if (m_pTemplatesScene == watched)
    {
        focusScene = m_pTemplatesScene;
        focusArea = tr("模板库");
    }
    else if (m_pAlbumsScene == watched)
    {
        focusScene = m_pAlbumsScene;
        focusArea = tr("相册集");
    }
    else if (m_pEditPage->m_pThumbsScene == watched && m_pEditPage->m_thumbsSceneFocused)
    {
        focusScene = m_pEditPage->m_pThumbsScene;
    }

    if (focusScene)
    {
        QPointF pos;

        if (QEvent::MouseButtonPress == event->type())
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent)
            {
                pos = mouseEvent->pos();
            }
        }
        else if (QEvent::GraphicsSceneMousePress == event->type())
        {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            if (mouseEvent)
            {
                pos = mouseEvent->scenePos();
            }
        }

        if (!pos.isNull() && !focusScene->itemAt(QPointF(pos - QPoint(3, 3)), PictureProxyWidget::getTransform()))
        {
            m_pEditPage->m_thumbsSceneFocused = false;

            for (int i = 0; i < PictureGraphicsScene::SceneType_End; i++)
            {
                //qDebug() << __FILE__ << __LINE__ << i << m_scensVector.size();
                if (m_scensVector.at(i))
                {
                    m_scensVector[i]->clearFocusSelection(true);
                }
            }

            return true;
        }
        else if (QEvent::KeyPress == event->type())
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent)
            {
                if (Qt::Key_Delete == keyEvent->key())
                {
                    int n = focusScene->selectedItems().size();

                    if (m_pEditPage != watched)
                    {
                        asking = QString("确定要从当前%1当中删除掉这 %2 张%3吗？").arg(focusArea).arg(n).arg(focusArea.left(2));
                    }
                    else
                    {
                        asking = QString("确定要从当前相册当中删除掉这 %2 张照片吗？").arg(n);
                    }

                    if (n && QMessageBox::AcceptRole == QMessageBox::question(this, tr("删除确认"), asking, tr("确定"), tr("取消")))
                    {
                        focusScene->updateScenes(m_scensVector);

                        if (m_pPhotosScene == focusScene || m_pTemplatesScene == focusScene)
                        {
                            focusScene->removeProxyWidgets(false, !m_pEditPage->isVisible() ? NULL : m_pEditPage);
                        }
                        else
                        {
                            focusScene->removeProxyWidgets(false);
                        }

                        if (!m_pEditPage->isVisible())
                        {
                            focusScene->adjustViewLayout();
                        }
                        else
                        {
                            focusScene->adjustViewLayout(m_pEditPage->getViewWidth());
                            m_pEditPage->updateAlbum();
                        }

                        noticeChanged();
                    }

                    return true;
                }
                else if ((keyEvent->modifiers() & Qt::ControlModifier) && Qt::Key_A == keyEvent->key())
                {
                    items = focusScene->items();
                    foreach (QGraphicsItem *item, items)
                    {
                        if ((proxyWidget = static_cast<PictureProxyWidget *>(item)))
                        {
                            proxyWidget->getChildWidget().updateBorder(true);
                            proxyWidget->setSelected(true);
                        }
                    }

                    return true;
                }
                else if (Qt::Key_Left == keyEvent->key())
                {
                    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "Qt::Key_Left";
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void TaskPageWidget::customEvent(QEvent *ce)
{
    PictureGraphicsScene *focusScene = static_cast<PictureGraphicsScene *>(PictureProxyWidget::getFocusScene());
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "focusScene:" << focusScene;
    if (!focusScene)
    {
        return;
    }

    if (CustomEvent_Item_Selected == ce->type())
    {
        if (m_pFocusScene != focusScene)
        {
            if (m_pFocusScene)
            {
                m_pFocusScene->clearFocusSelection(false);
            }

            m_pFocusScene = focusScene;
        }
    }

    if (CustomEvent_Item_Detached == ce->type())
    {
        QString focusArea;
        QString asking;

        if (m_pPhotosScene == focusScene)
        {
            focusArea = tr("照片库");
        }
        else
        {
            qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "ignore";
            return;
        }

        asking = tr("确定要从当前%1当中删除掉这张%2吗？").arg(focusArea).arg(focusArea.left(2));

        PictureProxyWidget *proxyWidget = NULL;
        GraphicsItemsList items = focusScene->selectedItems();

        foreach (QGraphicsItem *item, items)
        {
            proxyWidget = static_cast<PictureProxyWidget *>(item);
            if (proxyWidget && proxyWidget->isDetachable() &&
                QMessageBox::AcceptRole == QMessageBox::question(this, tr("删除确认"), asking, tr("确定"), tr("取消")))
            {
                noticeChanged();
                focusScene->removeProxyWidget(proxyWidget);
                focusScene->adjustViewLayout();
                break;
            }
        }

        if (m_pFocusScene != focusScene)
        {
            m_pFocusScene = focusScene;
        }
    }
}

void TaskPageWidget::detachItem(QGraphicsScene *scene, const QString &file)
{
    QString focusArea;
    QString asking;
    PictureGraphicsScene *focusScene = static_cast<PictureGraphicsScene *>(scene);

    if (m_pPhotosScene == focusScene)
    {
        focusArea = tr("照片库");
    }
    else if (m_pTemplatesScene == focusScene)
    {
        focusArea = tr("模板库");
    }
    else
    {
        return;
    }

    PhotoProxyWidget *proxyWidget = NULL;
    DraggableLabel *picLabel = NULL;
    GraphicsItemsList items = scene->items();

    foreach (QGraphicsItem *item, items)
    {
        if (NULL == (proxyWidget = static_cast<PhotoProxyWidget *>(item)) ||
            NULL == (picLabel = proxyWidget->getChildWidget().getPictureLabel()) ||
            file != picLabel->getPictureFile())
        {
            continue;
        }

        asking = tr("确定要从当前%1当中删除掉这张%2吗？").arg(focusArea).arg(focusArea.left(2));
        if (QMessageBox::AcceptRole == QMessageBox::question(this, tr("删除确认"), asking, tr("确定"), tr("取消")))
        {
            noticeChanged();
            focusScene->removeProxyWidget(proxyWidget);
            focusScene->adjustViewLayout();
            break;
        }
    }
}

void TaskPageWidget::on_collapsePushButton_clicked()
{
    QSize size = ui->photosGraphicsView->size();

    if (!m_bCollapsed)
    {
        ui->collapsePushButton->setText(tr("<"));
        ui->collapsePushButton->setToolTip(tr("显示右边栏"));
        ui->templatesGroupBox->hide();
        size.setWidth(size.width() + ui->templatesGroupBox->width());
    }
    else
    {
        ui->collapsePushButton->setText(tr(">"));
        ui->collapsePushButton->setToolTip(tr("隐藏右边栏"));
        ui->templatesGroupBox->show();
        size.setWidth(size.width() - ui->templatesGroupBox->width());
    }

    m_bCollapsed = !m_bCollapsed;
    ui->photosGraphicsView->resize(size);
    m_pPhotosScene->adjustViewLayout();
}

void TaskPageWidget::on_importPhotosPushButton_clicked()
{
    QStringList fileNames;
    FileParser fp(this);

    if (0 < fp.importFiles("photos_dir", tr("导入照片"), tr("照片文件(*.png *.jpg *.bmp)"), fileNames))
    {
        if (MAX_DELAY_PHOTOS_NUMBER >= fileNames.size())
        {
            foreach (const QString &fileName, fileNames)
            {
                if (m_pPhotosScene->filesList().contains(fileName, Qt::CaseInsensitive))
                {
                    continue;
                }

                int index = m_pPhotosScene->items().size() + 1;
                PhotoChildWidget *childWidget = new PhotoChildWidget(index, fileName, 0, Qt::ZAxis, 0, this);
                m_pPhotosScene->insertProxyWidget(index, new PhotoProxyWidget(childWidget), fileName);
                m_pPhotosScene->adjustItemPos();
                noticeChanged();
            }
        }
        else
        {
            m_pPhotosLoader->loadList(m_pPhotosScene->filesList(), fileNames);
            m_pPhotosLoader->begin();
        }
    }
}

void TaskPageWidget::importTemplate()
{
    QStringList fileNames("page.dat");
    FileParser fp(this);

    if (1 == fp.importFiles("templates_dir", tr("导入模板"), tr("模板文件(*.dat)"), fileNames, false))
    {
//        if (MAX_DELAY_TEMPLATES_NUMBER >= fileNames.size())
//        {
//            foreach (const QString &file, fileNames)
//            {
//                if (m_pTemplatesScene->filesList().contains(file, Qt::CaseInsensitive))
//                {
//                    continue;
//                }

//                int index = m_pTemplatesScene->items().size() + 1;
//                m_pTemplatesScene->insertProxyWidget(index, new TemplateProxyWidget(new TemplateChildWidget(index, file, 4, this)), file);
//                m_pTemplatesScene->adjustItemPos();
//                noticeChanged();
//            }
//        }
//        else
//        {
//            m_pTemplatesLoader->loadList(m_pTemplatesScene->filesList(), fileNames);
//            m_pTemplatesLoader->begin();
//        }


        int index = m_pTemplatesScene->items().size() + 1;
        QString fileName = QDir::toNativeSeparators(fileNames.first());
        TemplateChildWidget *childWidget = new TemplateChildWidget(index, fileName, 0, this);
        m_pTemplatesScene->insertProxyWidget(index, new TemplateProxyWidget(childWidget), fileName);
        m_pTemplatesScene->adjustItemPos();
        noticeChanged();

#if 0
        QString tmplFile = QDir::toNativeSeparators(fileNames.first());
        qDebug() << __FILE__ << __LINE__ << "template:" << tmplFile;
        //TemplateChildWidget::init(tmplFile);

        QVariantMap bases, size, attr;
        QVariantList tags, layers;
        FileParser fp(tmplFile);

        if (!fp.openTemplate(bases, size, tags, layers))
        {
            return;
        }

        QVariantMap::const_iterator iter = bases.constBegin();
        while (iter != bases.constEnd())
        {
            qDebug() << __FILE__ << __LINE__ << iter.key() << "=" << iter.value();
            ++iter;
        }

        qDebug() << __FILE__ << __LINE__ << "size:" << size["width"] << "x" << size["height"];

        foreach (const QVariant &tag, tags)
        {
            attr = tag.toMap();
            qDebug() << __FILE__ << __LINE__ << "tag:" << attr["name"] << "," << attr["type"];
        }

        foreach (const QVariant &layer, layers)
        {
            attr = layer.toMap();
            QVariantMap::const_iterator nested = attr.constBegin();
            while (nested != attr.constEnd())
            {
                if ("frame" == nested.key())
                {
                    QVariantMap frame = nested.value().toMap();
                    qDebug() << __FILE__ << __LINE__ << "frame:" << frame["height"] << "," << frame["width"] << "," << frame["x"]<< "," << frame["y"];
                }
                else
                {
                    qDebug() << __FILE__ << __LINE__ << "frame:" << nested.key() << "=" << nested.value();
                }

                ++nested;
            }
        }
#endif
    }
}

void TaskPageWidget::on_addAlbumPushButton_clicked()
{
    int count = m_pAlbumsScene->items().size() + 1;
    AlbumProxyWidget *proxyWidget = new AlbumProxyWidget(new AlbumChildWidget(count, this));
    m_pAlbumsScene->insertProxyWidget(count, proxyWidget);
    m_pAlbumsScene->adjustItemPos();
    noticeChanged();
}

void TaskPageWidget::onPreview(const QStringList &pictures, int current)
{
    if (m_pPreviewDlg)
    {
        m_pPreviewDlg->updateList(pictures, current);

        if (m_pPreviewDlg->isVisible())
        {
            m_pPreviewDlg->show();
        }
        else
        {
            m_pPreviewDlg->showMaximized();
        }
    }
}

void TaskPageWidget::onEdit(const ChildWidgetsMap &albumsMap, int current)
{
    enterEdit(true);
    m_pEditPage->updateViews(albumsMap, current);
}

bool TaskPageWidget::replace(PictureGraphicsScene::SceneType type,
                             const QString &current,
                             const QString &replaced)
{
    bool ok = false;

    if (PictureGraphicsScene::SceneType_Albums > type && current != replaced)
    {
        ProxyWidgetsMap &proxyWidgets = m_scensVector[type]->getProxyWidgetsMap();
        foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
        {
            DraggableLabel *picLabel = proxyWidget->getChildWidget().getPictureLabel();
            if (!picLabel || picLabel->getPictureFile().isEmpty())
            {
                continue;
            }

            /* Changes photo file attributes after replaced the current photo in thumbs scene */
            QVariantMap &belongings = picLabel->getBelongings();
            QString file = belongings["picture_file"].toString();
            int usedTimes = belongings["used_times"].toInt();

            if (current == file && 0 < usedTimes)
            {
                belongings["used_times"] = usedTimes - 1;
            }

            if (replaced == file || replaced == belongings["template_file"].toString())
            {
                belongings["used_times"] = usedTimes + 1;
            }

            if (usedTimes != belongings["used_times"].toInt())
            {
                picLabel->accept(ok = true);
            }
        }
    }

    return ok;
}

const TemplateChildWidget *TaskPageWidget::getTemplateWidget(const QString &tmplFile)
{
    TemplateChildWidget *childWidget = NULL;
    ProxyWidgetsMap &proxyWidgets = m_scensVector[PictureGraphicsScene::SceneType_Templates]->getProxyWidgetsMap();

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        if ((childWidget = static_cast<TemplateChildWidget *>(proxyWidget->getChildWidgetPtr())) &&
            tmplFile == childWidget->getTmplFile())
        {
            return childWidget;
        }
    }

    return childWidget;
}

void TaskPageWidget::enterEdit(bool enter)
{
    if (enter)
    {
        ui->collapsePushButton->hide();
        ui->photosGroupBox->hide();
        ui->templatesGroupBox->hide();
        ui->albumsGroupBox->hide();
        emit maxShow(true);
        m_pEditPage->show();
        m_pEditPage->adjustViewLayout();
    }
    else
    {
        emit maxShow(false);
        ui->collapsePushButton->show();
        ui->photosGroupBox->show();
        ui->templatesGroupBox->show();
        ui->albumsGroupBox->show();
        m_pEditPage->hide();
        adjustSize();
    }
}
