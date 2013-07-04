#include "EditPageWidget.h"
#include "ui_EditPageWidget.h"
#include "AlbumPageWidget.h"
#include "TaskPageWidget.h"
#include "child/ThumbChildWidget.h"
#include "defines.h"
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
    m_x(0),
    m_y(0),
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
                                              container);
    m_container->m_scensVector.insert(PictureGraphicsScene::SceneType_Thumbs, m_pThumbsScene);

    ui->photosGraphicsView->setScene(m_container->m_pPhotosScene);

    m_pTemplatePage = new TemplatePageWidget(true, m_container);
    m_pTemplatePage->getView()->setScene(m_container->m_pTemplatesScene);
    ui->mainHorizontalLayout->addWidget(m_pTemplatePage);

    on_editPushButton_clicked();

    connect(m_pTemplatePage, SIGNAL(replaced(QString,QString)), SLOT(onReplaced(QString,QString)));
}

EditPageWidget::~EditPageWidget()
{
    delete ui;
}

void EditPageWidget::onReplaced(const QString &current, const QString &replaced)
{
    Q_ASSERT(m_pAlbumWidget);

    if (m_pAlbumPage->isVisible())
    {
        qDebug() << __FILE__ << __LINE__ << current << replaced;
        m_pAlbumPage->compose();
    }
    else if (ui->photosGraphicsView->isVisible())
    {
        m_pAlbumPage->replace(current, replaced);
        m_container->replace(PictureGraphicsScene::SceneType_Photos, current, replaced);
        updateAlbum();
        m_container->noticeChanged();
    }
    else if (m_pTemplatePage->isVisible())
    {
        m_container->replace(PictureGraphicsScene::SceneType_Templates, current, replaced);
        m_pAlbumWidget->changeTemplate(m_pTemplatePage->getBelongings());
        switchPage(m_current);
        qDebug() << __FILE__ << __LINE__ << replaced;
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
            qDebug() << __FILE__ << __LINE__ << photoName << m_layerLabel->getFrame();
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

    if (m_pTemplatePage->isVisible())
    {
        return m_pTemplatePage->getView()->width();
    }

    return 0;
}

inline void EditPageWidget::adjustViewLayout()
{
//    if (m_bgdLabel->isVisible() && m_bgdLabel->hasPicture())
//    {
//        m_bgdLabel->setPixmap(QPixmap(m_bgdPic).scaled(m_bgdLabel->size(),
//                                                       Qt::KeepAspectRatio,
//                                                       Qt::SmoothTransformation));
//    }

    if (ui->photosGraphicsView->isVisible())
    {
        m_container->m_pPhotosScene->adjustViewLayout(ui->mainFrame->width());
    }

    if (m_pTemplatePage->isVisible())
    {
        m_container->m_pTemplatesScene->adjustViewLayout(m_pTemplatePage->getView()->width());
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
    if (m_pTemplatePage->isVisible())
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
            m_pThumbsScene->removeProxyWidget(proxyWidget);
            m_pThumbsScene->adjustViewLayout();
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

void EditPageWidget::updateAlbum()
{
    Q_ASSERT(m_pAlbumWidget);

    AlbumPhotos photosVector(PHOTOS_NUMBER);
    QStringList &photosList = m_pAlbumWidget->getPhotosList();
    DraggableLabels &photoLabels = m_pAlbumWidget->getPhotoLabels();
    ProxyWidgetsMap &thumbWidgetsMap = m_pThumbsScene->getProxyWidgets();

    /* Empty photos list */
    photosList.clear();

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        photoLabels.at(i)->reset();
    }

    foreach (PictureProxyWidget *proxyWidget, thumbWidgetsMap)
    {
        ThumbChildWidget &thumbWidget = static_cast<ThumbChildWidget &>(proxyWidget->getChildWidget());
        DraggableLabel *thumbLabel = thumbWidget.getPictureLabel();
        QPixmap pix = thumbLabel->getPicture();

        if (pix.isNull())
        {
            continue;
        }

        int id = thumbWidget.getId();
        photoLabels.at(id)->setPixmap(pix.scaled(QSize(45, 45), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QVariantMap belongings = thumbWidget.getBelongings();
        QString picFile = belongings["picture_file"].toString();
        qreal angle = belongings["rotation_angle"].toReal();
        Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
        int usedTimes = belongings["used_times"].toInt();
        photosVector[id] = QString("%1|%2|%3|%4").arg(picFile).arg(angle).arg(axis).arg(usedTimes);
        photosList.append(picFile);
        //qDebug() << __FILE__ << __LINE__ << id << photosVector[id];
    }

    //qDebug() << __FILE__ << __LINE__ << photosVector;
    m_pAlbumWidget->setPhotosVector(photosVector);
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

    m_pThumbsScene->removeProxyWidgets(true);
    m_pThumbsScene->getProxyWidgets().clear();
    m_pAlbumPage->clearLayers();

    ui->indexLabel->setText(tr("第%1页/共%2页").arg(index).arg(count));

    if ((m_pAlbumWidget = static_cast<AlbumChildWidget *>(m_albumsMap[index])))
    {
        int pid = 0, tid = 0;
        bool validTmpl = true;
        AlbumPhotos photosVector = m_pAlbumWidget->getPhotosVector();

        //qDebug() << __FILE__ << __LINE__ << photosVector;

        if (!m_pAlbumPage->loadLayers(*m_pAlbumWidget))
        {
            m_pAlbumPage->m_bgdLabel->loadPixmap(QPixmap(":/images/canvas.png"));
            validTmpl = false;
            qDebug() << __FILE__ << __LINE__ << "load layers failed!";
        }

        //qDebug() << __FILE__ << __LINE__ << m_pAlbumPage->geometry() << m_pAlbumPage->m_bgdLabel->geometry() << topLeft;

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

            if (validTmpl && m_pAlbumPage->loadPhoto(pid, photoFile, angle, axis))
            {
                pid++;
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
        m_pTemplatePage->changeTemplate(m_pAlbumWidget->getTmplLabel().getBelongings());
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
    m_pTemplatePage->hide();
}

void EditPageWidget::on_photoPushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->photoPushButton->setCheckable(true);
    m_pAlbumPage->hide();
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->show();
    m_pTemplatePage->hide();
    adjustViewLayout();
}

void EditPageWidget::on_templatePushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 0);
    ui->templatePushButton->setCheckable(true);
    m_pAlbumPage->hide();
    ui->photosGraphicsView->hide();
    ui->thumbsGraphicsView->hide();
    m_pTemplatePage->show();
    adjustViewLayout();
}

void EditPageWidget::on_backPushButton_clicked()
{
    Q_ASSERT(m_pAlbumWidget);

    m_pAlbumWidget->unselectSelf();
    m_container->m_pPhotosScene->clearFocusSelection(false);

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
        m_container->m_pAlbumsScene->removeProxyWidget(m_current);
        m_container->m_pAlbumsScene->adjustViewLayout();
        m_container->noticeChanged();

        PictureProxyWidget *proxyWidget = NULL;
        PictureChildWidget *childWidget = NULL;
        QList<QGraphicsItem *> items = m_container->m_pAlbumsScene->items();

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
