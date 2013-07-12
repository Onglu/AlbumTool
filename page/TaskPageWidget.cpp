#include "TaskPageWidget.h"
#include "ui_TaskPageWidget.h"
#include "StartupPageWidget.h"
#include "PreviewDialog.h"
#include "MainWindow.h"
#include "LoadingDialog.h"
#include "child/PhotoChildWidget.h"
#include "child/TemplateChildWidget.h"
#include "parser/json.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QtSql>
#include <QMovie>
#include <QVBoxLayout>
#include <QDateTime>

#define MAX_DELAY_PHOTOS_NUMBER     2
#define MAX_DELAY_TEMPLATES_NUMBER  2
#define MAX_DELAY_ALBUMS_NUMBER     2

using namespace QtJson;
using namespace TaskPage;

TaskPageWidget::TaskPageWidget(int tabId, const QString &taskFile, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskPageWidget),
    m_tabId(tabId),
    m_taskParser(taskFile, parent),
    m_photosLoader(new LoaderThread(ViewType_Photo)),
    m_templatesLoader(new LoaderThread(ViewType_Template)),
    m_albumsLoader(new LoaderThread(ViewType_Album)),
    m_loadingDlg(new LoadingDialog),
    m_collapsed(false),
    m_changed(false),
    m_focusScene(NULL),
    m_scensVector(GraphicsScenesVector(PictureGraphicsScene::SceneType_End))
{
    ui->setupUi(this);

    /* Setups graphics scenes */
    m_photosScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Grid,
                                              PictureGraphicsScene::SceneType_Photos,
                                              ui->photosGraphicsView,
                                              this);
    m_scensVector.insert(PictureGraphicsScene::SceneType_Photos, m_photosScene);

    m_templatePage = new TemplatePageWidget(false, this);
    ui->rightVerticalLayout->addWidget(m_templatePage);

    m_templatesScene = new PictureGraphicsScene(Qt::gray,
                                                 PictureGraphicsScene::LayoutMode_Grid,
                                                 PictureGraphicsScene::SceneType_Templates,
                                                 m_templatePage->getView(),
                                                 this);
    m_scensVector.insert(PictureGraphicsScene::SceneType_Templates, m_templatesScene);

    m_albumsScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Horizontality,
                                              PictureGraphicsScene::SceneType_Albums,
                                              ui->albumsGraphicsView,
                                              this);
    m_scensVector.insert(PictureGraphicsScene::SceneType_Albums, m_albumsScene);

    m_editPage = new EditPageWidget(this);
    ui->mainHorizontalLayout->addWidget(m_editPage);
    m_editPage->hide();
    connect(m_editPage, SIGNAL(editEntered(bool)), SLOT(enterEdit(bool)));

    m_previewDlg = new PreviewDialog(this);
    connect(m_previewDlg, SIGNAL(itemDetached(QGraphicsScene*,QString)), SLOT(detachItem(QGraphicsScene*,QString)));

    connect(&m_maker, SIGNAL(doing(int,QStringList)), SLOT(process(int,QStringList)), Qt::BlockingQueuedConnection);

    connect(m_photosLoader, SIGNAL(itemAdded(int,QString,qreal,Qt::Axis,int)),
            SLOT(addItem(int,QString,qreal,Qt::Axis,int)), Qt::BlockingQueuedConnection);
    connect(m_templatesLoader, SIGNAL(itemAdded(int,QString,int)),
            SLOT(addItem(int,QString,int)), Qt::BlockingQueuedConnection);
    connect(m_albumsLoader, SIGNAL(itemAdded(int,QStringList,QString,QVariantList)),
            SLOT(addItem(int,QStringList,QString,QVariantList)), Qt::BlockingQueuedConnection);

    connect(m_photosLoader, SIGNAL(done(uchar)), SLOT(ok(uchar)));
    connect(m_templatesLoader, SIGNAL(done(uchar)), SLOT(ok(uchar)));
    connect(m_albumsLoader, SIGNAL(done(uchar)), SLOT(ok(uchar)));

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
    m_photosLoader->disconnect(SIGNAL(itemAdded(int,QString,qreal,Qt::Axis,int)));
    delete m_photosLoader;

    m_templatesLoader->disconnect(SIGNAL(itemAdded(int,QString,int)));
    delete m_templatesLoader;

    m_albumsLoader->disconnect(SIGNAL(itemAdded(int,QStringList,QString,QVariantList)));
    delete m_albumsLoader;

    if (m_loadingDlg)
    {
        delete m_loadingDlg;
        m_loadingDlg = NULL;
    }

    delete ui;
}

void TaskPageWidget::noticeChanged()
{
    if (!m_changed)
    {
        m_changed = true;
        emit changed(m_tabId);
    }

    countLocations(PictureGraphicsScene::SceneType_Photos);
    countLocations(PictureGraphicsScene::SceneType_Albums);
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
            if (!m_changed)
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
    if (!m_changed)
    {
        return;
    }

    QVariantList photos, templates, albums;
    m_photosScene->getChanges(photos);
    m_templatesScene->getChanges(templates);
    m_albumsScene->getChanges(albums);

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
        m_changed = false;
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
        m_photosLoader->loadList(m_photosList);
        m_photosLoader->begin();
    }

//    if (MAX_DELAY_TEMPLATES_NUMBER > m_templatesList.size())
//    {
//        loadViewItems(m_templatesList, ViewType_Template);
//    }
//    else
    {
        m_templatesLoader->loadList(m_templatesList);
        m_templatesLoader->begin();
    }

//    if (MAX_DELAY_ALBUMS_NUMBER > m_albumsList.size())
//    {
//        loadViewItems(m_albumsList, ViewType_Album);
//    }
//    else
    {
        m_albumsLoader->loadList(m_albumsList);
        m_albumsLoader->begin();
    }

    if (m_photosLoader->isActive() || m_templatesLoader->isActive() || m_albumsLoader->isActive())
    {
        m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在加载..."));
    }
}

void TaskPageWidget::addItem(int index, const QString &file, qreal angle, Qt::Axis axis, int usedTimes)
{   
    PhotoChildWidget *childWidget = new PhotoChildWidget(index, file, angle, axis, usedTimes, this);
    m_photosScene->insertProxyWidget(index, new PhotoProxyWidget(childWidget), file);
    m_photosScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Photo:" << index << file << angle << axis << usedTimes;
}

void TaskPageWidget::addItem(int index, const QString &file, int usedTimes)
{
    TemplateChildWidget *childWidget = new TemplateChildWidget(index, file, usedTimes, this);
    m_templatesScene->insertProxyWidget(index, new TemplateProxyWidget(childWidget), file);
    m_templatesScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Template:" << index << file << usedTimes;
}

void TaskPageWidget::addItem(int index,
                             const QStringList &filesList,
                             const QString &file,
                             const QVariantList &changes)
{
    AlbumProxyWidget *proxyWidget = new AlbumProxyWidget(new AlbumChildWidget(index, filesList, file, changes, this));
    m_albumsScene->insertProxyWidget(index, proxyWidget);
    m_albumsScene->autoAdjust();
    //qDebug() << __FILE__ << __LINE__ << "Album:" << index << changes.size() << changes;
}

void TaskPageWidget::ok(uchar state)
{
    if (state)
    {
        if (EXPORT_PAGES != state)
        {
            if (!m_photosLoader->isActive() && !m_templatesLoader->isActive() && !m_albumsLoader->isActive())
            {
                if (LOAD_RECORDS == state)
                {
                    countLocations(PictureGraphicsScene::SceneType_Photos);
                    countLocations(PictureGraphicsScene::SceneType_Albums);
                }

                m_loadingDlg->showProcess(false);
            }

            if (!m_photosLoader->isActive() && !m_photosScene->done())
            {
                m_photosScene->ok(true);
            }

            if (!m_templatesLoader->isActive() && !m_templatesScene->done())
            {
                m_templatesScene->ok(true);
            }
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
                addItem(index, file, angle, axis, usedTimes);
            }
            else
            {
                addItem(index, file, usedTimes);
            }
        }
    }

    //qDebug("%s(%d): loads %d view items cost %d ms in total!", __FILE__ , __LINE__, view, tm.elapsed());
}

void TaskPageWidget::resizeEvent(QResizeEvent *)
{
    if (m_editPage->isHidden())
    {
        m_photosScene->adjustViewLayout();
        m_templatesScene->adjustViewLayout();
    }
}

bool TaskPageWidget::eventFilter(QObject *watched, QEvent *event)
{
    QString focusArea, asking;
    PictureGraphicsScene *focusScene = NULL;
    PictureProxyWidget *proxyWidget = NULL;
    GraphicsItemsList items;

    if (m_photosScene == watched)
    {
        focusScene = m_photosScene;
        focusArea = tr("照片库");
    }
    else if (m_templatesScene == watched)
    {
        focusScene = m_templatesScene;
        focusArea = tr("模板库");
    }
    else if (m_albumsScene == watched)
    {
        focusScene = m_albumsScene;
        focusArea = tr("相册");
    }
    else if (m_editPage->m_pThumbsScene == watched && m_editPage->m_thumbsSceneFocused)
    {
        //watched = NULL;
        focusScene = m_editPage->m_pThumbsScene;
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
            m_editPage->m_thumbsSceneFocused = false;

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

                    if (!focusArea.isEmpty())
                    {
                        asking = tr("确定要从当前%1当中删除掉这 %2 张%3吗？").arg(focusArea).arg(n).arg(focusArea.left(2));
                    }
                    else
                    {
                        asking = tr("确定要从当前相册页当中删除掉这 %2 张照片吗？").arg(n);
                    }

                    if (n && QMessageBox::AcceptRole == QMessageBox::question(this, tr("删除确认"), asking, tr("确定"), tr("取消")))
                    {
                        focusScene->updateScenes(m_scensVector);

                        if (!m_editPage->isVisible())
                        {
                            focusScene->removeProxyWidgets(false);
                            focusScene->adjustViewLayout();
                        }
                        else
                        {
                            focusScene->removeProxyWidgets(false, m_editPage);
                            focusScene->adjustViewLayout(m_editPage->getViewWidth());
                            m_editPage->updatePage();
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
            if (m_focusScene != focusScene)
            {
                if (m_focusScene)
                {
                    m_focusScene->clearFocusSelection(false);
                }

                m_focusScene = focusScene;
            }
        }
        else if (CustomEvent_Item_Detached == type)
        {
            QString focusArea;
            QString asking;

            if (m_photosScene == focusScene)
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

            if (m_focusScene != focusScene)
            {
                m_focusScene = focusScene;
            }
        }
        else if (CustomEvent_Item_Replaced == type)
        {
            //qDebug() << __FILE__ << __LINE__ << "current:" << current << "," << ", replaced:" << replaced;
        }
    }
    else if (CustomEvent_Load_BEGIN == type)
    {
        m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在加载..."));
    }
//    else if (CustomEvent_MAKE_BEGIN == type)
//    {
//        m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在生成相册..."));
//    }
}

void TaskPageWidget::detachItem(QGraphicsScene *scene, const QString &file)
{
    QString focusArea;
    QString asking;
    PictureGraphicsScene *focusScene = static_cast<PictureGraphicsScene *>(scene);

    if (m_photosScene == focusScene)
    {
        focusArea = tr("照片库");
    }
    else if (m_templatesScene == focusScene)
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

    if (!m_collapsed)
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

    m_collapsed = !m_collapsed;
    ui->photosGraphicsView->resize(size);
    m_photosScene->adjustViewLayout();
}

void TaskPageWidget::countLocations(PictureGraphicsScene::SceneType type)
{
    if (PictureGraphicsScene::SceneType_End <= type)
    {
        return;
    }

    int landscape = 0, portrait = 0;
    ProxyWidgetsMap proxyWidgets = m_scensVector[type]->getProxyWidgets();

    if (PictureGraphicsScene::SceneType_Photos == type)
    {
        foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
        {
            const PhotoChildWidget *childWidget = (PhotoChildWidget *)proxyWidget->getChildWidgetPtr();
            if (childWidget && !childWidget->usedTimes())
            {
                if (childWidget->getPictureLabel()->getOrientation())
                {
                    landscape++;
                }
                else
                {
                    portrait++;
                }
            }
        }

        QString info = tr("共导入 %1 张照片，未入册竖幅 %2 张，横幅 %3 张").arg(proxyWidgets.size()).arg(portrait).arg(landscape);
        ui->photosLabel->setText(info);
    }

    if (PictureGraphicsScene::SceneType_Albums == type)
    {
        int num = 0;
        //m_pictures.clear();

        if (proxyWidgets.isEmpty())
        {
            ui->createPushButton->setEnabled(false);
            ui->previewPushButton->setEnabled(false);
        }
        else
        {
            int count = 0;
            //QString taskFile = m_taskParser.getParsingFile(), outDir = taskFile.left(taskFile.length() - 5);

            foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
            {
                const AlbumChildWidget *childWidget = (AlbumChildWidget *)proxyWidget->getChildWidgetPtr();
                if (childWidget)
                {
                    if (childWidget->getData().isEmpty())
                    {
                        count++;
                        continue;
                    }

                    uchar locations[2] = {0};
                    int n = childWidget->getLocations(locations);
                    if (0 < n)
                    {
                        num += n;
                        portrait += locations[PORTRAIT_PICTURE];
                        landscape += locations[LANDSCAPE_PICTURE];
                    }
                    //qDebug() << __FILE__ << __LINE__ << childWidget->getIndex() << locations[0] << locations[1] << landscape << portrait;

//                    int from = childWidget->getIndex() - 1;
//                    QString picFile = !from ? tr("%1\\cover.png").arg(outDir) : tr("%1\\page%2.png").arg(outDir).arg(from);
//                    if (QFile::exists(picFile))
//                    {
//                        m_pictures << picFile;
//                    }
                }
            }

            //qDebug() << __FILE__ << __LINE__ << count;
            ui->createPushButton->setEnabled(!(count == proxyWidgets.size()));
            //ui->previewPushButton->setEnabled(!m_pictures.isEmpty());
        }

        QString info = tr("共有 %1 个空位，需要竖幅照片 %2 张，横幅照片 %3 张").arg(num).arg(portrait).arg(landscape);
        ui->albumsLabel->setText(info);
    }
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
                if (m_photosScene->getFilesList().contains(fileName, Qt::CaseInsensitive))
                {
                    continue;
                }

                int index = m_photosScene->items().size() + 1;
                PhotoChildWidget *childWidget = new PhotoChildWidget(index, fileName, 0, Qt::ZAxis, 0, this);
                m_photosScene->insertProxyWidget(index, new PhotoProxyWidget(childWidget), fileName);
                m_photosScene->adjustItemPos();
                noticeChanged();
            }
        }
        else
        {
            m_photosScene->ok(false);
            m_photosLoader->loadList(m_photosScene->getFilesList(), fileNames);
            m_photosLoader->begin();
        }
    }
}

void TaskPageWidget::importTemplates()
{
    QStringList fileNames;
    FileParser fp(this);

    if (0 < fp.importFiles("templates_dir", tr("导入模板"), tr("相册模板(*.xcmb)"), fileNames))
    {
        if (MAX_DELAY_TEMPLATES_NUMBER >= fileNames.size())
        {
            foreach (const QString &file, fileNames)
            {
                if (m_templatesScene->getFilesList().contains(file, Qt::CaseInsensitive))
                {
                    continue;
                }

                int index = m_templatesScene->items().size() + 1;
                m_templatesScene->insertProxyWidget(index,
                                                    new TemplateProxyWidget(new TemplateChildWidget(index, file, 0, this)),
                                                    file);
                m_templatesScene->adjustItemPos();
                noticeChanged();
            }
        }
        else
        {
            m_templatesScene->ok(false);
            m_templatesLoader->loadList(m_templatesScene->getFilesList(), fileNames);
            m_templatesLoader->begin();
        }
    }
}

void TaskPageWidget::on_addAlbumPushButton_clicked()
{
    int count = m_albumsScene->items().size() + 1;
    AlbumProxyWidget *proxyWidget = new AlbumProxyWidget(new AlbumChildWidget(count, this));
    m_albumsScene->insertProxyWidget(count, proxyWidget);
    m_albumsScene->adjustItemPos();
    noticeChanged();
}

void TaskPageWidget::on_createPushButton_clicked()
{
#if 1
    ProxyWidgetsMap proxyWidgets = m_albumsScene->getProxyWidgets();
    int count = 0, num = 0;
    uchar locations[2] = {0};

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        AlbumChildWidget *childWidget = static_cast<AlbumChildWidget *>(proxyWidget->getChildWidgetPtr());
        if (childWidget)
        {
            count += childWidget->getPhotosList().size();
            num += childWidget->getLocations(locations);
        }
    }

    QString asking = tr("本套相册共有 %1 页，入册照片 %2 张，相册中剩余 %3 个空位。确定要开始生成吗？").arg(proxyWidgets.size()).arg(count).arg(num);
    if (QMessageBox::RejectRole == QMessageBox::question(this, tr("操作提示"), asking, tr("确定"), tr("取消")))
    {
        return;
    }

    /* start make */
    QString fileName, taskDir, outDir, childDir, targetFile = m_taskParser.getParsingFile();

    outDir = targetFile.left(targetFile.length() - 5);
    TemplateChildWidget::deleteDir(outDir);

    childDir = tr("%1\\output").arg(outDir);
    taskDir = outDir.left(outDir.lastIndexOf(QDir::separator()));
    Converter::getFileName(targetFile, fileName, false);
    targetFile = tr("%1\\%2.xc").arg(taskDir).arg(fileName);

    if (QFile::exists(targetFile))
    {
        if (QMessageBox::RejectRole == QMessageBox::question(this,
                                                             tr("操作提示"),
                                                             tr("该目录下已经存在一个同名的相册包，继续生成将会替换旧的相册包，确定要继续生成吗？"),
                                                             tr("确定"),
                                                             tr("取消")))
        {
            return;
        }
        else
        {
            QFile::remove(targetFile);
        }
    }

    m_pictures.clear();
    m_package.clear();
    m_pages.clear();

    m_package.insert("ver", "1.0");
    m_package.insert("name", fileName);
    m_package.insert("pageCount", 0);
    m_package.insert("createTime", QDateTime::currentDateTime().toString("yyyyMMddhhmm"));

    m_tm.start();

    QStringList args;
    args << taskDir << outDir << childDir << fileName;
    m_maker.begin(args);

    QString info = tr("正在生成相册...");

    if (!m_loadingDlg || (m_loadingDlg && info != m_loadingDlg->getInfo()))
    {
        if (m_loadingDlg)
        {
            delete m_loadingDlg;
        }

        m_loadingDlg = new LoadingDialog;
    }

    m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), info);
#else
    // for test
    m_maker.begin(QStringList());
    m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在生成相册..."));
#endif
}

void TaskPageWidget::process(int index, const QStringList &args)
{
#if 0
    QString args, fileName, taskDir, outDir, childDir, targetFile = m_taskParser.getParsingFile();
    int count = 0;
    QVariantMap pack;
    QVariantList pages;
    ProxyWidgetsMap proxyWidgets = m_albumsScene->getProxyWidgets();

    //QCoreApplication::postEvent(this, new QEvent(CustomEvent_MAKE_BEGIN));

    outDir = targetFile.left(targetFile.length() - 5);
    TemplateChildWidget::deleteDir(outDir);

    childDir = tr("%1\\output").arg(outDir);
    taskDir = outDir.left(outDir.lastIndexOf(QDir::separator()));
    Converter::getFileName(targetFile, fileName, false);
    targetFile = tr("%1\\%2.xc").arg(taskDir).arg(fileName);

    if (QFile::exists(targetFile))
    {
        if (QMessageBox::RejectRole == QMessageBox::question(this,
                                                             tr("操作提示"),
                                                             tr("该目录下已经存在一个同名的相册包，继续生成将会替换旧的相册包，确定要继续生成吗？"),
                                                             tr("确定"),
                                                             tr("取消")))
        {
            return;
        }
        else
        {
            QFile::remove(targetFile);
        }
    }

    pack.insert("ver", "1.0");
    pack.insert("name", fileName);
    pack.insert("pageCount", 0);
    pack.insert("createTime", QDateTime::currentDateTime().toString("yyyyMMddhhmm"));

    m_pictures.clear();

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        AlbumChildWidget *childWidget = static_cast<AlbumChildWidget *>(proxyWidget->getChildWidgetPtr());
        if (childWidget && childWidget->output(childDir))
        {
            QVariantMap data = childWidget->getData();
            int from = childWidget->getIndex() - 1;
            QString picFile;

            if (!from)
            {
                pack.insert("cover", data);
                picFile = tr("%1\\cover.png").arg(outDir);
                QFile::copy(tr("%1\\cover\\preview.png").arg(childDir), picFile);
            }
            else
            {
                pages << data;
                picFile = tr("%1\\page%2.png").arg(outDir).arg(from);
                QFile::copy(tr("%1\\page%2\\preview.png").arg(childDir).arg(from), picFile);
            }

            m_pictures << picFile;

            count++;
        }
    }

    pack["pageCount"] = count;
    pack.insert("pages", pages);

    QFile jf(tr("%1\\package.dat").arg(childDir));
    if (jf.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QByteArray result = QtJson::serialize(pack);
        jf.write(result);
        jf.close();
    }

    QProcess tmaker;
    //QString arg = tr("%1%2.xc %3").arg(outDir.left(outDir.lastIndexOf(QDir::separator()) + 1)).arg(fileName).arg(outDir);
    args = tr("%1\\package.xc %2").arg(outDir).arg(childDir);
    TemplateChildWidget::useZip(tmaker, TemplateChildWidget::ZipUsageCompress, args, true);

    TemplateChildWidget::deleteDir(childDir);

    args = tr("%1\\%2.xc %3").arg(taskDir).arg(fileName).arg(outDir);
    TemplateChildWidget::useZip(tmaker, TemplateChildWidget::ZipUsageCompress, args, true);

    m_loadingDlg->showProcess(false);

    ui->previewPushButton->setEnabled(true);

#else

    //qDebug() << __FILE__ << __LINE__ << "now:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");

    QTime t;
    t.start();

#if 0
    while(t.elapsed() < 2000);
    //qDebug("Time elapsed: %d ms", t.elapsed());

    if (5 < index)
    {
        m_maker.end();
        m_loadingDlg->showProcess(false);
    }
#else
    QString taskDir = args.at(0), outDir = args.at(1), childDir = args.at(2), fileName = args.at(3);
    ProxyWidgetsMap proxyWidgets = m_albumsScene->getProxyWidgets();

    if (index > proxyWidgets.size())
    {
        m_maker.end();

        m_package["pageCount"] = m_pictures.size();
        m_package.insert("pages", m_pages);

        QFile jf(tr("%1\\package.dat").arg(childDir));
        if (jf.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QByteArray result = QtJson::serialize(m_package);
            jf.write(result);
            jf.close();
        }

        QProcess tmaker;

        TemplateChildWidget::useZip(tmaker,
                                    TemplateChildWidget::ZipUsageCompress,
                                    tr("\"%1\\package.xc\" \"%2\"").arg(outDir).arg(childDir),
                                    true);

        TemplateChildWidget::deleteDir(childDir);

        TemplateChildWidget::useZip(tmaker,
                                    TemplateChildWidget::ZipUsageCompress,
                                    tr("\"%1\\%2.xc\" \"%3\"").arg(taskDir).arg(fileName).arg(outDir),
                                    true);

        m_loadingDlg->showProcess(false);
        ui->previewPushButton->setEnabled(true);

        qDebug("\nTime elapsed: %d ms in total", m_tm.elapsed());

        return;
    }

    AlbumChildWidget *childWidget = static_cast<AlbumChildWidget *>(proxyWidgets[index]->getChildWidgetPtr());
    if (childWidget && childWidget->output(childDir))
    {
        QVariantMap data = childWidget->getData();
        QString picFile;

        index -= 1;

        if (!index)
        {
            m_package.insert("cover", data);
            picFile = tr("%1\\cover.png").arg(outDir);
            QFile::copy(tr("%1\\cover\\preview.png").arg(childDir), picFile);
        }
        else
        {
            m_pages << data;
            picFile = tr("%1\\page%2.png").arg(outDir).arg(index);
            QFile::copy(tr("%1\\page%2\\preview.png").arg(childDir).arg(index), picFile);
        }

        m_pictures << picFile;
    }
#endif

    qDebug("Time elapsed: %d ms", t.elapsed());
#endif

    //qDebug() << __FILE__ << __LINE__ << "now:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
}

void TaskPageWidget::on_previewPushButton_clicked()
{
    onPreview(m_pictures, 0);
}

void TaskPageWidget::onPreview(const QStringList &pictures, int current)
{
    m_previewDlg->updateList(pictures, current);

    if (m_previewDlg->isVisible())
    {
        m_previewDlg->show();
    }
    else
    {
        m_previewDlg->showMaximized();
    }
}

void TaskPageWidget::onEdit(const ChildWidgetsMap &albumsMap, int current)
{
    enterEdit(true);
    m_editPage->updateViews(albumsMap, current);
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
        m_editPage->show();
        m_editPage->adjustViewLayout();
        m_editPage->m_templatePage->setTags(m_templatePage->isImmediate(), m_templatePage->getTags());
    }
    else
    {
        emit maxShow(false);
        ui->collapsePushButton->show();
        ui->photosGroupBox->show();
        ui->templatesGroupBox->show();
        ui->albumsGroupBox->show();
        m_templatePage->setTags(m_editPage->m_templatePage->isImmediate(), m_editPage->m_templatePage->getTags());
        m_editPage->hide();
        adjustSize();
    }
}

bool TaskPageWidget::replace(PictureGraphicsScene::SceneType type,
                             const QString &current,
                             const QString &replaced)
{
    bool ok = false;

    //qDebug() << __FILE__ << __LINE__ << current << replaced;

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

TemplateChildWidget *TaskPageWidget::getTmplWidget(const QString &tmplFile) const
{
    ProxyWidgetsMap &proxyWidgets = m_templatesScene->getProxyWidgets();

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

void TaskPageWidget::onSearch(const QVariantMap &tags)
{
    ProxyWidgetsMap proxyWidgets = m_templatesScene->getProxyWidgets();

    m_templatesScene->clearProxyWidgets();

    if (tags.isEmpty())
    {
        m_templatesScene->addProxyWidgets(proxyWidgets);
        return;
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
    if (size)
    {
        PictureProxyWidget *proxyWidget = NULL;
        ProxyWidgetsMap::iterator iter = proxyWidgets.begin();

        while (iter != proxyWidgets.end())
        {
            if ((proxyWidget = static_cast<PictureProxyWidget *>(iter.value())))
            {
                DraggableLabel *picLabel = proxyWidget->getChildWidget().getPictureLabel();
                if (picLabel)
                {
                    //qDebug() << __FILE__ << __LINE__ << tmplPics.at(index) << picLabel->getPictureFile();

                    const QString picFile = picLabel->getPictureFile();
                    foreach (const QString &tmplPic, tmplPics)
                    {
                        if (picFile == tmplPic)
                        {
                            m_templatesScene->addProxyWidget(++index, proxyWidget);
                            m_templatesScene->adjustItemPos(true);
                            tmplPics.removeOne(tmplPic);
                            break;
                        }
                    }

                    if (tmplPics.isEmpty())
                    {
                        break;
                    }
                }
            }

            ++iter;
        }
    }
}
