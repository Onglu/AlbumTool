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
#include <QtSql>

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

    m_pTemplatePage = new TemplatePageWidget(false, this);
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
    connect(m_pAlbumsLoader, SIGNAL(itemAdded(int,QStringList,QString,QVariantList)),
            SLOT(addItem(int,QStringList,QString,QVariantList)), Qt::BlockingQueuedConnection);

    connect(m_pPhotosLoader, SIGNAL(loadFinished(uchar)), SLOT(finishLoaded(uchar)));
    connect(m_pTemplatesLoader, SIGNAL(loadFinished(uchar)), SLOT(finishLoaded(uchar)));
    connect(m_pAlbumsLoader, SIGNAL(loadFinished(uchar)), SLOT(finishLoaded(uchar)));

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

    m_pAlbumsLoader->disconnect(SIGNAL(itemAdded(int,QStringList,QString,QVariantList)));
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

    if (Task_SaveAs == mode)
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

    QVariantList photos, templates, albums;
    m_pPhotosScene->getChanges(photos);
    m_pTemplatesScene->getChanges(templates);
    m_pAlbumsScene->getChanges(albums);

    //qDebug() << __FILE__ << __LINE__ << templates;
    //qDebug() << __FILE__ << __LINE__ << m_templatesList;

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

//    if (MAX_DELAY_PHOTOS_NUMBER > m_photosList.size())
//    {
//        loadViewItems(m_photosList, ViewType_Photo);
//    }
//    else
    {
        m_pPhotosLoader->loadList(m_photosList);
        m_pPhotosLoader->begin();
    }

//    if (MAX_DELAY_TEMPLATES_NUMBER > m_templatesList.size())
//    {
//        loadViewItems(m_templatesList, ViewType_Template);
//    }
//    else
    {
        m_pTemplatesLoader->loadList(m_templatesList);
        m_pTemplatesLoader->begin();
    }

//    if (MAX_DELAY_ALBUMS_NUMBER > m_albumsList.size())
//    {
//        loadViewItems(m_albumsList, ViewType_Album);
//    }
//    else
    {
        m_pAlbumsLoader->loadList(m_albumsList);
        m_pAlbumsLoader->begin();
    }

    if (m_pPhotosLoader->isActive() || m_pTemplatesLoader->isActive() || m_pAlbumsLoader->isActive())
    {
        showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在加载..."));
    }
}

void TaskPageWidget::showProcess(bool show, QRect global, const QString &info)
{
    static QLabel *movieLabel = NULL, *textLabel = NULL;

    if (!m_pLoadingDlg)
    {
        m_pLoadingDlg = new QDialog(0, Qt::FramelessWindowHint);
        QVBoxLayout *vbl = new QVBoxLayout;

        movieLabel = new QLabel(m_pLoadingDlg);
        movieLabel->setFixedSize(126, 22);

        QMovie *movie = new QMovie(":/images/loading.gif");
        movie->setSpeed(100);
        movieLabel->setMovie(movie);

        textLabel = new QLabel(m_pLoadingDlg);
        QFont font = textLabel->font();
        font.setBold(true);
        textLabel->setFixedHeight(16);
        textLabel->setFont(font);
        textLabel->setStyleSheet("color:blue;");

        vbl->addWidget(movieLabel);
        vbl->addWidget(textLabel);
        vbl->setMargin(0);

        m_pLoadingDlg->setFixedSize(126, 38);
        m_pLoadingDlg->setLayout(vbl);
        m_pLoadingDlg->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (m_pLoadingDlg->isHidden() && show)
    {
        movieLabel->movie()->start();
        textLabel->setText(info);

        QPoint pos((global.width() - m_pLoadingDlg->width()) / 2, (global.height() - m_pLoadingDlg->height()) / 2);
        m_pLoadingDlg->move(global.topLeft() + pos);
        m_pLoadingDlg->exec();
    }

    if (m_pLoadingDlg->isVisible() && !show)
    {
        movieLabel->movie()->stop();
        m_pLoadingDlg->accept();
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
    //qDebug() << __FILE__ << __LINE__ << "Template:" << index << file << usedTimes;
}

void TaskPageWidget::addItem(int index,
                             const QStringList &filesList,
                             const QString &file,
                             const QVariantList &changes)
{
    AlbumProxyWidget *proxyWidget = new AlbumProxyWidget(new AlbumChildWidget(index, filesList, file, changes, this));
    m_pAlbumsScene->insertProxyWidget(index, proxyWidget);
    m_pAlbumsScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Album:" << index << changes.size() << changes;
}

void TaskPageWidget::finishLoaded(uchar state)
{
    if (state)
    {
        if (!m_pPhotosLoader->isActive() && !m_pTemplatesLoader->isActive() && !m_pAlbumsLoader->isActive())
        {
            showProcess(false);
        }

        if (!m_pPhotosLoader->isActive() && !m_pPhotosScene->loadFinished())
        {
            m_pPhotosScene->finishLoaded(true);
        }

        if (!m_pTemplatesLoader->isActive() && !m_pTemplatesScene->loadFinished())
        {
            m_pTemplatesScene->finishLoaded(true);
        }
    }
    else
    {
        noticeChanged();
        QCoreApplication::postEvent(this, new QEvent(CustomEvent_Load_BEGIN));
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
                //qDebug() << __FILE__ << __LINE__ << "Template >" << ":" << file << locations;
                addItem(index, file, usedTimes);
            }
        }
        else
        {
            //QStringList photosList = recordsMap["photos_list"].toStringList();
            //QString tmplFile = recordsMap["template_file"].toString();
            //qDebug() << __FILE__ << __LINE__ << "Albums:>" << index << "," << photosList << "," << tmplFile;
            //addItem(index, photosList, tmplFile);
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
                        asking = tr("确定要从当前%1当中删除掉这 %2 张%3吗？").arg(focusArea).arg(n).arg(focusArea.left(2));
                    }
                    else
                    {
                        asking = tr("确定要从当前相册当中删除掉这 %2 张照片吗？").arg(n);
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
    QEvent::Type type = ce->type();

    if (CustomEvent_Item_Selected <= type && CustomEvent_Item_Unknown > type)
    {
        PictureGraphicsScene *focusScene = static_cast<PictureGraphicsScene *>(PictureProxyWidget::getFocusScene());
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "focusScene:" << focusScene;
        if (!focusScene)
        {
            return;
        }

        if (CustomEvent_Item_Selected == type)
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
        else if (CustomEvent_Item_Detached == type)
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
    else if (CustomEvent_Load_BEGIN == type)
    {
        showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在加载..."));
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
            m_pPhotosScene->finishLoaded(false);
            m_pPhotosLoader->loadList(m_pPhotosScene->filesList(), fileNames);
            m_pPhotosLoader->begin();
        }
    }
}

void TaskPageWidget::importTemplates()
{
#ifdef FROM_PACKAGE
    QStringList fileNames;
#else
    QStringList fileNames("page.dat");
#endif

    FileParser fp(this);

    if (0 < fp.importFiles("templates_dir", tr("导入模板"), tr("相册模板(*.xcmb)"), fileNames/*, false*/))
    {
        if (MAX_DELAY_TEMPLATES_NUMBER >= fileNames.size())
        {
            foreach (const QString &file, fileNames)
            {
                if (m_pTemplatesScene->filesList().contains(file, Qt::CaseInsensitive))
                {
                    continue;
                }

                int index = m_pTemplatesScene->items().size() + 1;
                m_pTemplatesScene->insertProxyWidget(index,
                                                     new TemplateProxyWidget(new TemplateChildWidget(index, file, 0, this)),
                                                     file);
                m_pTemplatesScene->adjustItemPos();
                noticeChanged();
            }
        }
        else
        {
            m_pTemplatesScene->finishLoaded(false);
            m_pTemplatesLoader->loadList(m_pTemplatesScene->filesList(), fileNames);
            m_pTemplatesLoader->begin();
        }
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

void TaskPageWidget::enterEdit(bool enter)
{
    if (enter)
    {
        emit maxShow(true);
        ui->collapsePushButton->hide();
        ui->photosGroupBox->hide();
        ui->templatesGroupBox->hide();
        ui->albumsGroupBox->hide();
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

bool TaskPageWidget::replace(PictureGraphicsScene::SceneType type,
                             const QString &current,
                             const QString &replaced)
{
    bool ok = false;

    qDebug() << __FILE__ << __LINE__ << current << replaced;

    if (PictureGraphicsScene::SceneType_Albums > type && current != replaced)
    {
        ProxyWidgetsMap &proxyWidgets = m_scensVector[type]->getProxyWidgets();
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

            //qDebug() << __FILE__ << __LINE__ << file << usedTimes;

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

            //qDebug() << __FILE__ << __LINE__ << file << usedTimes;
        }
    }

    return ok;
}

TemplateChildWidget *TaskPageWidget::getTemplateWidget(const QString &tmplFile) const
{
    ProxyWidgetsMap &proxyWidgets = m_pTemplatesScene->getProxyWidgets();

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        TemplateChildWidget *childWidget = static_cast<TemplateChildWidget *>(proxyWidget->getChildWidgetPtr());
        if (childWidget && tmplFile == childWidget->getTmplFile())
        {
           return childWidget;
        }
    }

    return NULL;
}

void TaskPageWidget::onSearch(bool immediate, bool inner, const QVariantMap &tags)
{
    int width = 0;

    if (inner)
    {
        m_pTemplatePage->updateTags(immediate, tags);
        width = m_pEditPage->m_pTemplatePage->getView()->width();
    }
    else
    {
        m_pEditPage->m_pTemplatePage->updateTags(immediate, tags);
        width = m_pTemplatePage->getView()->width();
    }

    QSqlQuery query;
    QVariantMap::const_iterator iter = tags.constBegin();
    QVector<int> tids;
    int n, pageType = 0;
    QString value, sql, pageId = m_taskParser.getPageId();

    /* Get the id number of templates which meet the searching conditions */
    while (iter != tags.constEnd())
    {
        //qDebug() << __FILE__ << __LINE__ << iter.key() << iter.value().toString();
        if ("pagetype" != iter.key())
        {
            value = iter.value().toString();
            if (value.isEmpty())
            {
                ++iter;
                continue;
            }

            if (tr("风格") == iter.key())
            {
                sql = tr("select id from tproperty where ptype='%1' and name like '%%2%'").arg(iter.key()).arg(value);
            }
            else
            {
                sql = tr("select id from tproperty where ptype='%1' and name='%2'").arg(value).arg(iter.key());
            }

            int tid, pid = SqlHelper::getId(sql);
            //qDebug() << __FILE__ << __LINE__ << sql << "," << pid;

            if (pid)
            {
                if (tids.isEmpty()) // Get query resluts
                {
                    query.exec(QString("select templateid from template_property where propertyid=%1").arg(pid));
                    while (query.next())
                    {
                        tid = query.value(0).toInt();
                        if (tid && !tids.contains(tid))
                        {
                            tids.append(tid);
                        }
                    }
                }
                else // Flitter
                {
                    for (int i = tids.size() - 1; i >= 0; i--)
                    {
                        /* Remove the template which doesn't meet the template properties */
                        sql = QString("select templateid from template_property where templateid=%1 and propertyid=%2").arg(tids[i]).arg(pid);
                        tid = SqlHelper::getId(sql);
                        //qDebug() << __FILE__ << __LINE__ << sql << "," << tid << i << tids.size();
                        if (!tid)
                        {
                            tids.remove(i);
                        }
                    }
                    //qDebug() << __FILE__ << __LINE__ << tids;
                }
            }
        }
        else
        {
            pageType = iter.value().toInt();
        }

        ++iter;
    }

    QStringList tmplPics;
    if ((n = tids.size()))
    {
        for (int i = 0; i < n; i++)
        {
            sql = QString("select fileurl, page_type, page_id from template where id=%1").arg(tids[i]);
            query.exec(sql);
            while (query.next())
            {
                if (pageType == query.value(1).toInt() && pageId == query.value(2).toString())
                {
                    tmplPics << query.value(0).toString();
                }
            }
        }
    }
    else
    {
        sql = QString("select fileurl from template where page_type=%1 and page_id='%2'").arg(pageType).arg(pageId);
        query.exec(sql);
        while (query.next())
        {
            tmplPics << query.value(0).toString();
        }
    }

    //qDebug() << __FILE__ << __LINE__ << tmplPics;

    int index = 0, size = tmplPics.size();
    ProxyWidgetsMap proxyWidgets = m_pTemplatesScene->getProxyWidgets();
    m_pTemplatesScene->clearProxyWidgets();

    if (size)
    {
        PictureProxyWidget *proxyWidget = NULL;
        ProxyWidgetsMap::iterator iter = proxyWidgets.begin();

        while (iter != proxyWidgets.end())
        {
            if ((proxyWidget = static_cast<PictureProxyWidget *>(iter.value())))
            {
                DraggableLabel *picLabel = proxyWidget->getChildWidget().getPictureLabel();
                if (tmplPics.at(index) == picLabel->getPictureFile())
                {
                    m_pTemplatesScene->addProxyWidget(++index, proxyWidget);
                    m_pTemplatesScene->adjustItemPos(true);
                    //qDebug() << __FILE__ << __LINE__ << size << index << m_pTemplatesScene->getResultWidgets().size();

                    if (size <= index)
                    {
                        break;
                    }

                    iter = proxyWidgets.begin();
                    continue;
                }
            }

            ++iter;
        }
    }
}
