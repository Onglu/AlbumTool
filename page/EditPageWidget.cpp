#include "EditPageWidget.h"
#include "ui_EditPageWidget.h"
#include "AlbumPageWidget.h"
#include "TaskPageWidget.h"
#include "child/ThumbChildWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMovie>

#define SHOW_PROGRESSBAR    0

extern QRect g_appRect;

EditPageWidget::EditPageWidget(TaskPageWidget *container) :
    QWidget(0, Qt::WindowCloseButtonHint),
    ui(new Ui::EditPageWidget),
    m_container(container),
    m_layerLabel(NULL),
    m_pAlbumWidget(NULL),
    m_current(0),
    m_thumbsSceneFocused(false)
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    ui->resetPushButton->hide();
    ui->angleLineEdit->setValidator(new QIntValidator(-180, 180, this));

    // 初始化编辑页
    m_pAlbumPage = new AlbumPageWidget(PhotoLayer::VisiableImgTypeScreen, ui->mainFrame);
    ui->mainHorizontalLayout->insertWidget(0, m_pAlbumPage);

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        PhotoLayer *layer = m_pAlbumPage->m_layerLabels[i];
        connect(layer, SIGNAL(clicked(QPoint,QPoint)), SLOT(clickPicture(QPoint,QPoint)));
        connect(layer, SIGNAL(replaced(QString,QString)), SLOT(onReplaced(QString,QString)));
    }

    // 添加视图
    m_pThumbsScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Horizontality,
                                              PictureGraphicsScene::SceneType_Thumbs,
                                              ui->thumbsGraphicsView,
                                              m_container);
    m_container->m_scensVector.insert(PictureGraphicsScene::SceneType_Thumbs, m_thumbsScene);

    ui->photosGraphicsView->setScene(m_container->m_photosScene);

    // 添加模板页
    m_templatePage = new TemplatePageWidget(true, m_container);
    m_templatePage->getView()->setScene(m_container->m_templatesScene);
    ui->mainHorizontalLayout->addWidget(m_templatePage);
    connect(m_templatePage, SIGNAL(replaced(QString,QString)), SLOT(onReplaced(QString,QString)));

    exec(false);

#if SHOW_PROGRESSBAR
    m_loading = new QWidget(this)/*(0, Qt::FramelessWindowHint)*/;

    QLabel *movieLabel = new QLabel(m_loading);
    movieLabel->setFixedSize(126, 22);

    QMovie *movie = new QMovie(":/images/loading.gif");
    movie->setSpeed(100);
    movieLabel->setMovie(movie);

    QLabel *textLabel = new QLabel(m_loading);
    QFont font = textLabel->font();
    font.setBold(true);
    textLabel->setFixedHeight(16);
    textLabel->setFont(font);
    textLabel->setStyleSheet("color:blue;");

    QVBoxLayout *vbl = new QVBoxLayout;
    vbl->addWidget(movieLabel);
    vbl->addWidget(textLabel);
    vbl->setMargin(0);

    m_loading->setFixedSize(126, 38);
    m_loading->setLayout(vbl);
    m_loading->setAttribute(Qt::WA_TranslucentBackground);

    movieLabel->movie()->start();
    textLabel->setText(tr("正在加载..."));
    m_loading->hide();

    connect(&m_processor, SIGNAL(timeout()), SLOT(end()));
#endif
}

EditPageWidget::~EditPageWidget()
{
#if SHOW_PROGRESSBAR
    delete m_loading;
#endif

    delete ui;
}

void EditPageWidget::exec(bool open)
{
    const int top = 22;
    const int height = g_appRect.height() - top;

    setFixedSize(g_appRect.width(), height);
    setGeometry(0, top, g_appRect.width(), height);

    on_editPushButton_clicked();

    if (open)
    {
        show();
    }
}

void EditPageWidget::onReplaced(const QString &current, const QString &replaced)
{
    Q_ASSERT(m_pAlbumWidget);

    //qDebug() << __FILE__ << __LINE__ << current << replaced;

    if (m_pAlbumPage->isVisible())
    {
        PhotoLayer::getReplaced(m_layerLabel);
        if (!m_layerLabel)
        {
            return;
        }

        // 响应并处理相册编辑页的照片替换操作
        QString layerId = m_layerLabel->getLayerId();
        QVariantList &photosInfo = m_pAlbumWidget->getPhotosInfo();
        bool replaceable = false;

        //qDebug() << __FILE__ << __LINE__ << layerId << current << replaced;

        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QVariantMap info = photosInfo[i].toMap();
            if (info.isEmpty())
            {
                continue;
            }

            QString photoFile = info["picture_file"].toString();
            QVariantList records = info["used_records"].toList();

            if (!replaceable && current == photoFile)
            {
                int num = records.size();
                if (!num)
                {
                    qDebug() << __FILE__ << __LINE__ << "no used records:" << info;
                    continue;
                }

                if (1 < num)
                {
                    m_pAlbumWidget->removeUsedRecord(records, layerId);
                }
                else
                {
                    records.clear();
                }

                info.insert("used_records", records);
                photosInfo[i] = info;
                replaceable = true;
                i = -1;

                continue;
            }

            if (replaceable && replaced == photoFile)
            {
                if (!m_pAlbumWidget->addUsedRecord(records, layerId))
                {
                    return;
                }

                //qDebug() << __FILE__ << __LINE__ << layerId << records;
                info.insert("used_records", records);
                photosInfo[i] = info;

                break;
            }
        }

        //qDebug() << __FILE__ << __LINE__ << photosInfo << "\n";
        //qDebug() << __FILE__ << __LINE__ << m_pAlbumWidget->getPhotosInfo();

        m_pAlbumPage->replace(*m_pAlbumWidget, getThumbWidget(replaced), m_layerLabel);
        m_layerLabel = NULL;
    }
    else if (ui->photosGraphicsView->isVisible())
    {
        //qDebug() << __FILE__ << __LINE__ << current << replaced;

        const ThumbChildWidget *childWidget = getThumbWidget(current);
        if (!childWidget)
        {
            return;
        }

        // 响应并处理相册照片页的替换操作
        m_pAlbumPage->replace(*m_pAlbumWidget, childWidget);
        m_pAlbumWidget->replace(current, childWidget->getBelongings());
        m_container->replace(PictureGraphicsScene::SceneType_Photos, current, replaced);
        m_container->noticeChanged();
    }
    else if (m_templatePage->isVisible())
    {
        // 响应并处理相册模板页的替换操作
        m_container->replace(PictureGraphicsScene::SceneType_Templates, current, replaced);
        m_pAlbumWidget->changeTemplate(m_templatePage->getBelongings());
        switchPage(m_current);
    }
}

void EditPageWidget::clickPicture(QPoint wpos, QPoint epos)
{
    m_startPos = QPoint(0, 0);

    // 响应并处理照片控件的点击动作
    for (int i = PHOTOS_NUMBER - 1; i >= 0; i--)
    {
        PhotoLayer *layer = m_pAlbumPage->m_layerLabels[i];
//        if (layer->getPhotoFile().isEmpty())
//        {
//            continue;
//        }

        //QRect location = layer->getVisiableRect(PhotoLayer::VisiableRectTypeFixed);
        //qDebug() << __FILE__ << __LINE__ << i << wpos << epos << location << location.contains(epos) << layer->getPhotoFile();
        if (/*!layer->getPhotoFile().isEmpty() &&*/
                layer->getVisiableRect(PhotoLayer::VisiableRectTypeFixed).contains(epos))
        {
            m_startPos.rx() = wpos.x() + ui->mainFrame->x();
            m_startPos.ry() = wpos.y() + ui->mainFrame->y();

            if (m_layerLabel != layer)
            {
                m_layerLabel = layer;
            }

            layer->setMoveable(true);
            m_pAlbumPage->m_bgdLabel->updateBorder(BgdLayer::Press, m_layerLabel->getVisiableRect(PhotoLayer::VisiableRectTypeFixed));
            enableButtons(true);

            return;
        }
    }

    // 更新界面
    if (m_startPos.isNull())
    {
        m_pAlbumPage->m_bgdLabel->updateBorder(BgdLayer::Leave);
        enableButtons(false);
        m_layerLabel = NULL;
    }
}

void EditPageWidget::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton != event->button())
    {
        return;
    }

    // 处理外部的鼠标点击操作
    QPoint pos = event->pos();

    m_layerLabel = NULL;

    if (m_pAlbumPage->m_bgdLabel == childAt(pos))
    {
        pos.rx() -= m_pAlbumPage->m_bgdLabel->x() + ui->mainFrame->x();
        pos.ry() -= m_pAlbumPage->m_bgdLabel->y() + ui->mainFrame->y();
        if (0 < pos.x() && 0 < pos.y())
        {
            for (int i = PHOTOS_NUMBER - 1; i >= 0; i--)
            {
                PhotoLayer *label = m_pAlbumPage->m_layerLabels[i];
                if (/*!label->getPhotoFile().isEmpty() &&*/
                        label->getVisiableRect(PhotoLayer::VisiableRectTypeFixed).contains(pos))
                {
                    qDebug() << __FILE__ << __LINE__ << event->pos() << pos << label->getVisiableRect(PhotoLayer::VisiableRectTypeFixed);
                    m_layerLabel = label;
                    break;
                }
            }
        }
    }

    // 根据点击作用控件来决定如何来更新当前编辑界面
    if (!m_layerLabel)
    {
        setCursor(Qt::ArrowCursor);
        enableButtons(false);
        m_pAlbumPage->m_bgdLabel->updateBorder(BgdLayer::Leave);
    }
    else
    {
        setCursor(Qt::OpenHandCursor);
        m_layerLabel->setMoveable(true);
        m_pAlbumPage->m_bgdLabel->updateBorder(BgdLayer::Press, m_layerLabel->getVisiableRect(PhotoLayer::VisiableRectTypeFixed));
        m_startPos = event->pos();
        enableButtons(true);
    }
}

void EditPageWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 处理外部的鼠标移动操作
    QPoint pos = event->pos();
    if (QApplication::startDragDistance() <= (pos - m_startPos).manhattanLength()
        && m_layerLabel && m_layerLabel->isMoveable())
    {
        //qDebug() << __FILE__ << __LINE__ << m_startPos << event->pos() << event->pos() - m_startPos;
        m_layerLabel->movePhoto(QPoint(pos.x() - m_startPos.x(), pos.y() - m_startPos.y()));
        m_startPos = pos;
    }
}

void EditPageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 处理外部的鼠标释放操作
    if (m_pAlbumWidget && Qt::LeftButton == event->button() && m_layerLabel && m_layerLabel->isMoveable())
    {
        bool moved = m_layerLabel->hasMoved();

        setCursor(Qt::ArrowCursor);
        m_layerLabel->setMoveable(false);
        m_pAlbumPage->m_bgdLabel->updateBorder(BgdLayer::Releas);
#if 1
        if (moved)
        {
            updatePage();
        }
#endif
    }
}

int EditPageWidget::getViewWidth() const
{
    // 根据当前视图页类型来获取相应视图的宽度
    if (ui->photosGraphicsView->isVisible())
    {
        return ui->mainFrame->width();
    }

    if (m_templatePage->isVisible())
    {
        return m_templatePage->getView()->width();
    }

    return 0;
}

void EditPageWidget::adjustViewLayout()
{
//    if (m_bgdLabel->isVisible() && m_bgdLabel->hasPicture())
//    {
//        m_bgdLabel->setPixmap(QPixmap(m_bgdPic).scaled(m_bgdLabel->size(),
//                                                       Qt::KeepAspectRatio,
//                                                       Qt::SmoothTransformation));
//    }

    if (ui->photosGraphicsView->isVisible())
    {
        m_container->m_photosScene->adjustViewLayout(ui->mainFrame->width());
    }

    if (m_templatePage->isVisible())
    {
        m_container->m_templatesScene->adjustViewLayout(m_templatePage->getView()->width());
    }
}

inline void EditPageWidget::adjustThumbsHeight()
{
    int height = 162;

    if (PHOTOS_NUMBER == m_thumbsScene->getProxyWidgets().size() && 1072 > ui->mainFrame->width())
    {
        height = 178;
    }

    ui->thumbsGraphicsView->setMinimumHeight(height);
    ui->thumbsGraphicsView->setMaximumHeight(height);
}

void EditPageWidget::resizeEvent(QResizeEvent *event)
{
    if (m_pAlbumWidget && isVisible())
    {
        adjustViewLayout();
        adjustThumbsHeight();
        return;
    }

    QWidget::resizeEvent(event);
}

void EditPageWidget::removeThumbs(const QString &picFile)
{
    if (!m_pAlbumWidget || m_templatePage->isVisible())
    {
        return;
    }

    DraggableLabel *thumbLabel = NULL;
    ProxyWidgetsMap &proxyWidgets = m_thumbsScene->getProxyWidgets();

    // 移除指定文件名称的视图控件
    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        if ((thumbLabel = proxyWidget->getChildWidget().getPictureLabel())
             && picFile == thumbLabel->getPictureFile())
        {
            //qDebug() << __FILE__ << __LINE__ << "remove" << picFile;
            m_thumbsScene->removeProxyWidget(proxyWidget);
            m_thumbsScene->adjustViewLayout();
            m_pAlbumWidget->removePhoto(picFile);
            m_pAlbumPage->removePhoto(picFile);

            QVariantList photosInfo = m_pAlbumWidget->getPhotosInfo();
            int pid = m_pAlbumPage->loadPhotos(*m_pAlbumWidget, photosInfo, m_pAlbumWidget->getTotalUsedTimes());
            m_pAlbumPage->compose(pid);

            break;
        }
    }
}

void EditPageWidget::updateViews(const ChildWidgetsMap &albumsMap, int current)
{
    // 更新当前相册编辑页的界面
    bool enable = 1 != albumsMap.size();
    ui->previousPushButton->setEnabled(enable);
    ui->nextPushButton->setEnabled(enable);
    m_albumsMap = albumsMap;
    switchPage(m_current = current);
}

const ThumbChildWidget *EditPageWidget::getThumbWidget(const QString &picFile) const
{
    ProxyWidgetsMap proxyWidgets = m_thumbsScene->getProxyWidgets();

    // 获取指定文件名的视图控件
    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        const ThumbChildWidget *childWidget = static_cast<ThumbChildWidget *>(proxyWidget->getChildWidgetPtr());
        DraggableLabel *picLabel = childWidget->getPictureLabel();

        if (picLabel && picFile == picLabel->getPictureFile())
        {
            return childWidget;
        }
    }

    return NULL;
}

void EditPageWidget::updatePage()
{
    // 当外部发生编辑操作时，更新可视区域，并对照片层进行重新合成，同时更新照片属性信息
    if (m_pAlbumWidget && m_layerLabel)
    {
        m_layerLabel->updateRect();
        m_pAlbumPage->m_bgdLabel->compose();

        QString photoName;
        Converter::getFileName(m_layerLabel->getPictureFile(), photoName, false);

        //qDebug() << __FILE__ << __LINE__ << m_layerLabel->getPhotoFile() << m_layerLabel->getFrame() << m_layerLabel->getAngle();
        m_pAlbumWidget->changePhoto(photoName,
                                    m_layerLabel->getFrame(),
                                    m_layerLabel->getOpacity(),
                                    m_layerLabel->getAngle());
    }
}

void EditPageWidget::switchPage(int index)
{
    int count = m_albumsMap.size();

    m_thumbsSceneFocused = false;

    if (0 >= index || index > count)
    {
        return;
    }

    // 清理操作
    m_pAlbumPage->clearLayers();
    m_layerLabel = NULL;

    enableButtons(false);
    ui->deletePushButton->setEnabled(1 != index);

    m_thumbsScene->removeProxyWidgets(true);
    m_thumbsScene->getProxyWidgets().clear();

    ui->indexLabel->setText(tr("第%1页/共%2页").arg(index).arg(count));

    if ((m_pAlbumWidget = static_cast<AlbumChildWidget *>(m_albumsMap[index])))
    {
        int pid = 0, tid = 0;
        QVariantList &photosInfo = m_pAlbumWidget->getPhotosInfo();
        QStringList photosList;

//        QTime t;
//        t.start();

        //qDebug() << __FILE__ << __LINE__ << photosInfo;

//        QPoint offset((this->width() - m_loading->width()) / 2, (this->height() - m_loading->height()) / 2);
//        m_loading->move(this->geometry().topLeft() + offset);
//        m_loading->show();
//        m_processor.start(100);
        //m_processor.singleShot(3000, this, SLOT(end()));

        //while(t.elapsed() < 200);

        //qDebug() << __FILE__ << __LINE__ << m_pAlbumPage->geometry() << m_pAlbumPage->m_bgdLabel->geometry();

        if (!m_pAlbumPage->loadLayers(*m_pAlbumWidget))
        {
            m_pAlbumPage->m_bgdLabel->loadPixmap(QPixmap(":/images/canvas.png"));
            m_pAlbumPage->m_bgdLabel->move(QPoint((ui->mainFrame->width() - m_pAlbumPage->m_bgdLabel->getSize().width()) / 2, 0));
            qDebug() << __FILE__ << __LINE__ << "load layers failed, the count of all layers is" << m_pAlbumPage->m_layers.size()
                     << ", the count of photo layers is" << m_pAlbumPage->m_photoLayers.size();
        }
        else
        {
            pid = m_pAlbumPage->loadPhotos(*m_pAlbumWidget, photosInfo, m_pAlbumWidget->getTotalUsedTimes());
            //qDebug() << __FILE__ << __LINE__ << pid;
            m_pAlbumPage->compose(pid);
        }

        // 创建视图控件
        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QVariantMap info = photosInfo[i].toMap();
            if (info.isEmpty())
            {
                continue;
            }

            QString photoFile = info["picture_file"].toString();
            qreal angle = info["rotation_angle"].toReal();
            Qt::Axis axis = (Qt::Axis)info["rotation_axis"].toInt();
            //qDebug() << __FILE__ << __LINE__ << i << photoFile;

            ThumbChildWidget *childWidget = new ThumbChildWidget(++tid, DRAGGABLE_PHOTO, photoFile, angle, axis, m_container);
            m_thumbsScene->insertProxyWidget(i + 1, new ThumbProxyWidget(childWidget), photoFile);
            childWidget->setId(i);
            photosList.append(photoFile);

            connect(childWidget, SIGNAL(itemSelected()), SLOT(selectThumb()));
            connect(childWidget, SIGNAL(itemReplaced(QString,QString)), SLOT(onReplaced(QString,QString)));
            //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i << index << photoFile << photoInfo.at(3);
        }

        if (!m_thumbsScene->isEmpty())
        {
            adjustThumbsHeight();
            m_thumbsScene->adjustViewLayout();
        }

        // 更新模板数据
        ThumbChildWidget::updateList(photosList);
        m_templatePage->changeTemplate(m_pAlbumWidget->getTmplLabel().getBelongings());

        //while(t.elapsed() < 3000);

        //qDebug() << __FILE__ << __LINE__ << t.elapsed();
        //m_processor.stop();
        //m_loading->close();
        //m_loading->hide();
    }
}

void EditPageWidget::on_editPushButton_clicked()
{
    // 切换到编辑页
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->editPushButton->setCheckable(true);
    m_pAlbumPage->show();
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->hide();
    m_templatePage->hide();
}

void EditPageWidget::on_photoPushButton_clicked()
{
    // 切换到照片页
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->photoPushButton->setCheckable(true);
    m_pAlbumPage->hide();
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->show();
    m_templatePage->hide();
    adjustViewLayout();
}

void EditPageWidget::on_templatePushButton_clicked()
{
    // 切换到模板页
    ui->verticalLayout->setContentsMargins(9, 9, 9, 0);
    ui->templatePushButton->setCheckable(true);
    m_pAlbumPage->hide();
    ui->photosGraphicsView->hide();
    ui->thumbsGraphicsView->hide();
    m_templatePage->show();
    adjustViewLayout();
}

void EditPageWidget::on_backPushButton_clicked()
{
    Q_ASSERT(m_pAlbumWidget);

    m_pAlbumWidget->switchView(false);
    m_container->m_photosScene->clearFocusSelection(false);

    emit editEntered(false);
}

void EditPageWidget::end()
{
#if SHOW_PROGRESSBAR
    static int i = 0;

    if (20 < ++i)
    {
        i = 0;
        m_loading->close();
        m_processor.stop();
    }

    qDebug() << __FILE__ << __LINE__ << m_loading->geometry() << i;

//    if (!m_processor.isActive())
//    {
//        m_loading->close();
//        return;
//    }

//    QPoint offset((this->width() - m_loading->width()) / 2, (this->height() - m_loading->height()) / 2);
//    m_loading->move(this->geometry().topLeft() + offset);
//    m_loading->show();
#endif
}

void EditPageWidget::on_previousPushButton_clicked()
{
    if (0 >= m_current - 1)
    {
        m_current = m_albumsMap.size();
    }
    else
    {
        m_current--;
    }

    switchPage(m_current);
}

void EditPageWidget::on_nextPushButton_clicked()
{
    if (m_albumsMap.size() < m_current + 1)
    {
        m_current = 1;
    }
    else
    {
        m_current++;
    }

    switchPage(m_current);
}

void EditPageWidget::on_deletePushButton_clicked()
{
    // 删除相册页操作
    if (QMessageBox::AcceptRole == QMessageBox::question(this, tr("删除确认"), tr("确定要从当前相册集当中删除掉此相册页吗？"), tr("确定"), tr("取消")))
    {
        m_container->m_albumsScene->removeProxyWidget(m_current);
        m_container->m_albumsScene->adjustViewLayout();
        m_container->noticeChanged();

        PictureProxyWidget *proxyWidget = NULL;
        PictureChildWidget *childWidget = NULL;
        QList<QGraphicsItem *> items = m_container->m_albumsScene->items();

        m_albumsMap.clear();

        foreach (QGraphicsItem *item, items)
        {
            if ((proxyWidget = static_cast<PictureProxyWidget *>(item)) && (childWidget = &proxyWidget->getChildWidget()))
            {
                m_albumsMap.insert(childWidget->getIndex(), childWidget);
            }
        }

        int count = m_albumsMap.size();
        m_current = count < m_current ? 1 : count;
        switchPage(m_current);
    }
}
#if 1
void EditPageWidget::on_zoomInPushButton_clicked()
{
    zoomAction(*ui->zoomOutPushButton, true);
}

void EditPageWidget::on_zoomOutPushButton_clicked()
{
    zoomAction(*ui->zoomOutPushButton, false);
}
#endif
void EditPageWidget::on_resetPushButton_clicked()
{
//    if (m_pAlbumWidget)
//    {
//        m_bgdLabel->compose(VISIABLE_IMG_ORIGINAL, "C:\\Users\\Onglu\\Desktop\\test\\Composed_Final.jpg");
//    }

    if (m_layerLabel)
    {
        m_layerLabel->zoomAction(1);
        m_layerLabel->updateRect();
        m_pAlbumPage->m_bgdLabel->compose();
    }
}

void EditPageWidget::on_mirroredPushButton_clicked()
{
    if (m_layerLabel)
    {
        QString angle = ui->angleLineEdit->text();
        if (!angle.isEmpty())
        {
            ui->angleLineEdit->setText(QString("%1").arg(-1 * angle.toDouble()));
        }

        m_layerLabel->rotateAction(180.0f, Qt::YAxis);
        updatePage();
    }
}

#if 0
void EditPageWidget::on_zoomInPushButton_pressed()
{
    if (m_layerLabel && !m_layerLabel->zoomAction(1.10))
    {
        ui->zoomInPushButton->setEnabled(false);
    }
}

void EditPageWidget::on_zoomInPushButton_released()
{
    releaseButton(*ui->zoomInPushButton);
}

void EditPageWidget::on_zoomOutPushButton_pressed()
{
    if (m_layerLabel && !m_layerLabel->zoomAction(0.90))
    {
        ui->zoomOutPushButton->setEnabled(false);
    }
}

void EditPageWidget::on_zoomOutPushButton_released()
{
    releaseButton(*ui->zoomOutPushButton);
}
#endif

inline void EditPageWidget::releaseButton(const QPushButton &button)
{
    if (m_layerLabel && button.isEnabled())
    {
        m_layerLabel->updateRect();
        m_pAlbumPage->m_bgdLabel->compose();
        //qDebug() << __FILE__ << __LINE__ << m_layerLabel->getFrame();
    }
}

inline void EditPageWidget::enableButtons(bool enable)
{
    // 更新控件按钮的使能状态
    ui->zoomInPushButton->setEnabled(enable);
    ui->zoomOutPushButton->setEnabled(enable);
    ui->mirroredPushButton->setEnabled(enable);
    ui->angleLabel->setEnabled(enable);
    ui->angleLineEdit->setEnabled(enable);

    if (enable)
    {
        ui->angleLineEdit->setText(QString("%1").arg(m_layerLabel->getAngle()));
    }
    else
    {
        ui->angleLineEdit->clear();
    }
}

inline void EditPageWidget::zoomAction(QPushButton &button, bool in)
{
    // 处理照片层的缩放操作
    if (m_layerLabel && m_layerLabel->zoomAction(in ? 1.10 : 0.90))
    {
        m_layerLabel->updateRect();
        m_pAlbumPage->m_bgdLabel->compose();

        QString photoName;
        Converter::getFileName(m_layerLabel->getPictureFile(), photoName, false);
        m_pAlbumWidget->changePhoto(photoName,
                                    m_layerLabel->getFrame(),
                                    m_layerLabel->getOpacity(),
                                    m_layerLabel->getAngle());

        //qDebug() << __FILE__ << __LINE__ << m_layerLabel->getFrame();

        if (in && !ui->zoomOutPushButton->isEnabled())
        {
            ui->zoomOutPushButton->setEnabled(true);
        }

        if (!in && !ui->zoomInPushButton->isEnabled())
        {
            ui->zoomInPushButton->setEnabled(true);
        }

        if (!ui->resetPushButton->isEnabled())
        {
            ui->resetPushButton->setEnabled(true);
        }
    }
    else
    {
        button.setEnabled(false);
    }
}

void EditPageWidget::on_angleLineEdit_returnPressed()
{
    // 处理照片层的旋转操作
    if (m_layerLabel)
    {
        m_layerLabel->rotateAction(ui->angleLineEdit->text().toInt());
        updatePage();
    }
}
