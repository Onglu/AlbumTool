#include "EditPageWidget.h"
#include "ui_EditPageWidget.h"
#include "AlbumPageWidget.h"
#include "TaskPageWidget.h"
#include "child/ThumbChildWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>

#define ALBUM_PAGE  1

EditPageWidget::EditPageWidget(TaskPageWidget *container) :
    QWidget(0),
    ui(new Ui::EditPageWidget),
    m_container(container),
    m_layerLabel(NULL),
    m_pAlbumWidget(NULL),
    m_current(0),
    m_thumbsSceneFocused(false)
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    m_pAlbumPage = new AlbumPageWidget(PhotoLayer::VisiableImgTypeScreen, ui->mainFrame);
    ui->mainHorizontalLayout->insertWidget(0, m_pAlbumPage);

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        PhotoLayer *layer = m_pAlbumPage->m_layerLabels[i];
        connect(layer, SIGNAL(clicked(PhotoLayer&,QPoint)), SLOT(onClicked(PhotoLayer&,QPoint)));
        connect(layer, SIGNAL(replaced(QString,QString)), SLOT(onReplaced(QString,QString)));
    }

    m_pThumbsScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Horizontality,
                                              PictureGraphicsScene::SceneType_Thumbs,
                                              ui->thumbsGraphicsView,
                                              m_container);
    m_container->m_scensVector.insert(PictureGraphicsScene::SceneType_Thumbs, m_pThumbsScene);

    ui->photosGraphicsView->setScene(m_container->m_photosScene);

    m_templatePage = new TemplatePageWidget(true, m_container);
    m_templatePage->getView()->setScene(m_container->m_templatesScene);
    ui->mainHorizontalLayout->addWidget(m_templatePage);

    on_editPushButton_clicked();

    connect(m_templatePage, SIGNAL(replaced(QString,QString)), SLOT(onReplaced(QString,QString)));
}

EditPageWidget::~EditPageWidget()
{
    delete ui;
}

void EditPageWidget::onReplaced(const QString &current, const QString &replaced)
{
    Q_ASSERT(m_pAlbumWidget);

    //qDebug() << __FILE__ << __LINE__ << current << replaced;

    if (m_pAlbumPage->isVisible())
    {
        AlbumPhotos &photosVector = m_pAlbumWidget->getPhotosVector();
        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QStringList photoInfo = photosVector[i].split(TEXT_SEP);
            if (PHOTO_ATTRIBUTES != photoInfo.size())
            {
                continue;
            }

            QString photoFile = photoInfo.at(0);
            qreal angle = photoInfo.at(1).toDouble();
            Qt::Axis axis = (Qt::Axis)photoInfo.at(2).toInt();
            int usedTimes = photoInfo.at(3).toInt();

            if (current == photoFile)
            {
                if (0 < usedTimes)
                {
                    usedTimes--;
                }

                photosVector[i] = QString("%1|%2|%3|%4").arg(current).arg(angle).arg(axis).arg(usedTimes);
            }

            if (replaced == photoFile)
            {
                photosVector[i] = QString("%1|%2|%3|%4").arg(replaced).arg(angle).arg(axis).arg(usedTimes + 1);
            }
        }
        qDebug() << __FILE__ << __LINE__ << photosVector;

        m_pAlbumPage->replace(*m_pAlbumWidget, getThumbWidget(replaced), PhotoLayer::getReplaced(m_layerLabel));
        m_layerLabel = NULL;
    }
    else if (ui->photosGraphicsView->isVisible())
    {
        qDebug() << __FILE__ << __LINE__ << current << replaced;

        const ThumbChildWidget *childWidget = getThumbWidget(current);
        if (!childWidget)
        {
            return;
        }

//        m_layerLabel = NULL;
//        if (m_pAlbumPage->replace(childWidget, m_layerLabel))
//        {
//            QString photoName;
//            Converter::getFileName(m_layerLabel->getPictureFile(), photoName, false);
//            qDebug() << __FILE__ << __LINE__ << photoName << m_layerLabel->getFrame();
//            m_pAlbumWidget->changePhoto(photoName,
//                                        m_layerLabel->getFrame(),
//                                        m_layerLabel->getOpacity(),
//                                        m_layerLabel->getAngle());
//            m_layerLabel = NULL;
//        }

        m_pAlbumPage->replace(*m_pAlbumWidget, childWidget);
        m_pAlbumWidget->replace(current, childWidget->getBelongings());
        m_container->replace(PictureGraphicsScene::SceneType_Photos, current, replaced);
        m_container->noticeChanged();
    }
    else if (m_templatePage->isVisible())
    {
        m_container->replace(PictureGraphicsScene::SceneType_Templates, current, replaced);
        m_pAlbumWidget->changeTemplate(m_templatePage->getBelongings());
        QCoreApplication::postEvent(this, new QEvent(CustomEvent_Item_Replaced));
    }
}

void EditPageWidget::onClicked(PhotoLayer &label, QPoint pos)
{
    if (label.isMoveable())
    {
        m_pAlbumPage->m_bgdLabel->enterCopiedRect(true, label.getVisiableRect(PhotoLayer::VisiableRectTypeFixed));
        m_layerLabel = &label;
        m_startPos = pos;
        enableButtons(true);
    }
}

void EditPageWidget::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && m_layerLabel && m_layerLabel != childAt(event->pos()))
    {
        enableButtons(false);
        m_layerLabel = NULL;
    }
}

void EditPageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (QApplication::startDragDistance() <= (event->pos() - m_startPos).manhattanLength()
        && m_layerLabel && m_layerLabel->isMoveable())
    {
        QPoint pos = m_layerLabel->geometry().topLeft();
        pos.rx() += event->pos().x() - m_startPos.x();
        pos.ry() += event->pos().y() - m_startPos.y();
        m_layerLabel->movePhoto(QPoint(event->pos().x() - m_startPos.x(), event->pos().y() - m_startPos.y()));
        m_startPos = event->pos();
    }
}

void EditPageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pAlbumWidget && Qt::LeftButton == event->button() && m_layerLabel && m_layerLabel->isMoveable())
    {
        bool moved = m_layerLabel->hasMoved();

        m_layerLabel->setMoveable(false);
        m_pAlbumPage->m_bgdLabel->enterCopiedRect(false);

        if (moved)
        {
            m_pAlbumPage->m_bgdLabel->compose();

            QString photoName;
            Converter::getFileName(m_layerLabel->getPictureFile(), photoName, false);
            qDebug() << __FILE__ << __LINE__ << photoName << m_layerLabel->getOpacity();
            m_pAlbumWidget->changePhoto(photoName,
                                        m_layerLabel->getFrame(),
                                        m_layerLabel->getOpacity(),
                                        m_layerLabel->getAngle());
        }
    }
}

int EditPageWidget::getViewWidth() const
{
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

    if (PHOTOS_NUMBER == m_pThumbsScene->getProxyWidgets().size() && 1072 > ui->mainFrame->width())
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
    ProxyWidgetsMap &proxyWidgets = m_pThumbsScene->getProxyWidgets();

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        if ((thumbLabel = proxyWidget->getChildWidget().getPictureLabel())
             && picFile == thumbLabel->getPictureFile())
        {
            //qDebug() << __FILE__ << __LINE__ << "remove" << picFile;
            m_pThumbsScene->removeProxyWidget(proxyWidget);
            m_pThumbsScene->adjustViewLayout();
            m_pAlbumPage->removePhoto(picFile);
            m_pAlbumWidget->removePhoto(picFile);
            break;
        }
    }
}

void EditPageWidget::updateViews(const ChildWidgetsMap &albumsMap, int current)
{
    bool enable = 1 != albumsMap.size();
    ui->previousPushButton->setEnabled(enable);
    ui->nextPushButton->setEnabled(enable);
    m_albumsMap = albumsMap;
    switchPage(m_current = current);
}

const ThumbChildWidget *EditPageWidget::getThumbWidget(const QString &picFile) const
{
    ProxyWidgetsMap proxyWidgets = m_pThumbsScene->getProxyWidgets();

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
    if (m_pAlbumWidget)
    {
        QStringList photosList;
        Converter::v2l(m_pAlbumWidget->getPhotosVector(), photosList);
        m_pAlbumPage->compose(m_pAlbumPage->loadPhotos(photosList));
    }
}

bool EditPageWidget::switchPage(int index)
{
    bool ok = false;
    int count = m_albumsMap.size();

    m_thumbsSceneFocused = false;

    if (0 >= index || index > count)
    {
        return ok;
    }

    ui->deletePushButton->setEnabled(1 != index);

    m_pThumbsScene->removeProxyWidgets(true);
    m_pThumbsScene->getProxyWidgets().clear();
    m_pAlbumPage->clearLayers();

    ui->indexLabel->setText(tr("第%1页/共%2页").arg(index).arg(count));

    if ((m_pAlbumWidget = static_cast<AlbumChildWidget *>(m_albumsMap[index])))
    {
        int pid = 0, tid = 0, num = m_pAlbumWidget->getUsedNum(), locations = m_pAlbumWidget->getLocations();
        bool validTmpl = true;
        AlbumPhotos &photosVector = m_pAlbumWidget->getPhotosVector();

        //qDebug() << __FILE__ << __LINE__ << photosVector;

        if (!m_pAlbumPage->loadLayers(*m_pAlbumWidget))
        {
            m_pAlbumPage->m_bgdLabel->loadPixmap(QPixmap(":/images/canvas.png"));
            m_pAlbumPage->m_bgdLabel->move(QPoint((ui->mainFrame->width() - m_pAlbumPage->m_bgdLabel->getSize().width()) / 2, 0));
            validTmpl = false;
            qDebug() << __FILE__ << __LINE__ << "load layers failed:" << m_pAlbumPage->m_layers.size() << m_pAlbumPage->m_photoLayers.size();
        }

        //qDebug() << __FILE__ << __LINE__ << m_pAlbumPage->geometry() << m_pAlbumPage->m_bgdLabel->geometry();

        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QStringList photoInfo = photosVector[i].split(TEXT_SEP);
            if (PHOTO_ATTRIBUTES != photoInfo.size())
            {
                continue;
            }

            QString photoFile = photoInfo.at(0);
            qreal angle = photoInfo.at(1).toDouble();
            Qt::Axis axis = (Qt::Axis)photoInfo.at(2).toInt();
            int usedTimes = photoInfo.at(3).toInt();

            if (validTmpl)
            {
                if (!num)
                {
                    if (m_pAlbumPage->loadPhoto(pid, photoFile, angle, axis, tid + 1))
                    {
                        photosVector[i] = QString("%1|%2|%3|1").arg(photoFile).arg(angle).arg(axis);
                        pid++;
                    }
                    qDebug() << __FILE__ << __LINE__ << num << pid << photosVector[i];
                }
                else
                {
                    if (num > pid)
                    {
                        for (int j = 0; j < usedTimes; j++)
                        {
                            if (m_pAlbumPage->loadPhoto(pid, photoFile, angle, axis, tid + 1, num))
                            {
                                pid++;
                            }
                        }
                    }
                    else
                    {
                        if (num < locations)
                        {
                            if (m_pAlbumPage->loadPhoto(pid, photoFile, angle, axis, tid + 1, num))
                            {
                                photosVector[i] = QString("%1|%2|%3|1").arg(photoFile).arg(angle).arg(axis);
                                pid++;
                            }
                        }
                    }
                }

                //qDebug() << __FILE__ << __LINE__ << num << pid << photosVector[i];
            }

            ThumbChildWidget *childWidget = new ThumbChildWidget(++tid, DRAGGABLE_PHOTO, photoFile, angle, axis, m_container);
            m_pThumbsScene->insertProxyWidget(i + 1, new ThumbProxyWidget(childWidget), photoFile);
            childWidget->setId(i);

            connect(childWidget, SIGNAL(itemSelected(bool)), SLOT(selectThumb(bool)));
            connect(childWidget, SIGNAL(itemReplaced(QString,QString)), SLOT(onReplaced(QString,QString)));

            //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i << index << photoFile << photoInfo.at(3);
        }

        m_pAlbumPage->compose(pid);

        if (!m_pThumbsScene->isEmpty())
        {
            adjustThumbsHeight();
            m_pThumbsScene->adjustViewLayout();
        }

        ThumbChildWidget::updateList(m_pAlbumWidget->getPhotosList());
        m_templatePage->changeTemplate(m_pAlbumWidget->getTmplLabel().getBelongings());

        ok = true;
    }

    return ok;
}

void EditPageWidget::on_editPushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->editPushButton->setCheckable(true);
    m_pAlbumPage->show();
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->hide();
    m_templatePage->hide();
}

void EditPageWidget::on_photoPushButton_clicked()
{
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

    m_pAlbumWidget->unselectSelf();
    m_container->m_photosScene->clearFocusSelection(false);

    emit editEntered(false);
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

        if (!switchPage(m_current))
        {
            m_container->enterEdit(false);
        }
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
        m_layerLabel->flipAction();
        m_layerLabel->updateRect();
        m_pAlbumPage->m_bgdLabel->compose();
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
    ui->zoomInPushButton->setEnabled(enable);
    ui->zoomOutPushButton->setEnabled(enable);
    ui->mirroredPushButton->setEnabled(enable);
}

inline void EditPageWidget::zoomAction(QPushButton &button, bool in)
{
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
