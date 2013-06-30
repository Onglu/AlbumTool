#include "EditPageWidget.h"
#include "ui_EditPageWidget.h"
#include "TaskPageWidget.h"
#include "child/ThumbChildWidget.h"
//#include "wrapper/PhotoLayer.h"
//#include "wrapper/BgdLayer.h"
#include "defines.h"
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>

#define NOT_LOADED  1

EditPageWidget::EditPageWidget(TaskPageWidget *container) :
    QWidget(0),
    ui(new Ui::EditPageWidget),
    m_container(container),
    m_pAlbumWidget(NULL),
    m_current(0),
    m_x(0),
    m_y(0),
    m_thumbsSceneFocused(false),
    m_layerLabels(LabelsVector(PHOTOS_NUMBER)),
    m_layerLabel(NULL),
    m_bgdLabel(NULL)
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    m_bgdLabel = new BgdLayer(ui->mainFrame);
    ui->mainHorizontalLayout->insertWidget(0, m_bgdLabel);

#if NOT_LOADED
    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        PhotoLayer *layer = new PhotoLayer(ui->mainFrame);
        m_layerLabels.insert(i, layer);
        connect(layer, SIGNAL(refresh(PhotoLayer&)), SLOT(onRefreshed(PhotoLayer&)));
        connect(layer, SIGNAL(clicked(PhotoLayer&,QPoint)), SLOT(onClicked(PhotoLayer&,QPoint)));
    }
#endif

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

void EditPageWidget::onRefreshed(PhotoLayer &label)
{
    m_layerLabel = &label;
    m_bgdLabel->enterCopiedRect(true, label.visiableRect(PhotoLayer::VisiableRectTypeFixed));
    m_bgdLabel->compose(VISIABLE_IMG_SCREEN);
    m_bgdLabel->enterCopiedRect(false);

    //QString photoName;
    qDebug() << __FILE__ << __LINE__ << m_layerLabel->getPictureFile() << m_layerLabel->getFrame();
//    m_pAlbumWidget->changePhotoLayer(Converter::fileName(m_layerLabel->getPictureFile(), photoName),
//                                     m_layerLabel->getFrame(),
//                                     m_layerLabel->getOpacity(),
//                                     m_layerLabel->getAngle());
}

void EditPageWidget::onClicked(PhotoLayer &label, QPoint pos)
{
    if (label.isMoveable())
    {
        m_bgdLabel->enterCopiedRect(true, label.visiableRect(PhotoLayer::VisiableRectTypeFixed));
        m_layerLabel = &label;
        m_startPos = pos;
        enableButtons(true);
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_startPos << pos;
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
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << (event->pos() - m_startPos).manhattanLength();
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
        m_bgdLabel->enterCopiedRect(false);

        if (moved)
        {
            m_bgdLabel->compose(VISIABLE_IMG_SCREEN);

            QString photoName;
            //qDebug() << __FILE__ << __LINE__ << file << m_layerLabel->getFrame();
            m_pAlbumWidget->changePhotoLayer(Converter::fileName(m_layerLabel->getPictureFile(), photoName),
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
    swicth(m_current = current);
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

    m_pAlbumWidget->setViewsList(photosVector, m_pAlbumWidget->getTmplLabel().getPictureFile());
}

void EditPageWidget::onReplaced(const QString &current, const QString &replaced)
{
    Q_ASSERT(m_pAlbumWidget);

    if (m_bgdLabel->isVisible())
    {
        qDebug() << __FILE__ << __LINE__ << current << replaced;
    }
    else if (ui->photosGraphicsView->isVisible())
    {
        m_container->replace(PictureGraphicsScene::SceneType_Photos, current, replaced);
        updateAlbum();
        m_container->noticeChanged();
    }
    else if (m_pTemplatePage->isVisible())
    {
        m_container->replace(PictureGraphicsScene::SceneType_Templates, current, replaced);
        m_pAlbumWidget->changeTmplFile(m_pTemplatePage->getBelongings());
        swicth(m_current);
        qDebug() << __FILE__ << __LINE__ << replaced;
    }
}

void EditPageWidget::updateLayers()
{
    Q_ASSERT(m_pAlbumWidget);

    //qDebug() << __FILE__ << __LINE__ << m_pAlbumWidget->getTmplFile();
    TemplateChildWidget *tmplWidget = m_container->getTemplateWidget(m_pAlbumWidget->getTmplFile());
    if (!tmplWidget)
    {
        m_bgdLabel->loadPixmap(QPixmap(":/images/canvas.png"));
        return;
    }

    QString bgdPic, maskFile;
    QSize bgdSize = m_pAlbumWidget->getSize();
    QVariantMap photoLayer, maskLayer, pictures = tmplWidget->loadPictures();
    QVariantList photoLayers, maskLayers, layers = m_pAlbumWidget->getLayers();

    foreach (const QVariant &layer, layers)
    {
        QVariantMap embellishLayer, data = layer.toMap();
        QVariantMap frame = data["frame"].toMap();
        int width = frame["width"].toInt();
        int height = frame["height"].toInt();
        int x = frame["x"].toInt();
        int y = frame["y"].toInt();
        QString fileName = data["id"].toString() + ".png";

        if (bgdSize.width() == width && bgdSize.height() == height && x == width / 2 && y == height / 2)
        {
            bgdPic = fileName;
        }

        qreal opacity = data["opacity"].toReal();
        qreal angle = data["rotation"].toReal();
        int type = data["type"].toInt();

        if (LT_Photo == type)
        {
            int pos = fileName.lastIndexOf(".png");
            fileName.replace(pos, 4, PIC_FMT);
        }

        QByteArray ba = pictures[fileName].toByteArray();
//        QPixmap pix;
//        bool ret = pix.loadFromData(ba);
//        qDebug() << __FILE__ << __LINE__ << fileName << ret << pix.size();

        if (0 == type)
        {
            if (bgdPic == fileName)
            {
                QPixmap bgdPix;
                if (!bgdPix.loadFromData(ba))
                {
                    continue;
                }

                m_bgdLabel->setActualSize(bgdSize);
                m_bgdLabel->loadPixmap(bgdPix);
                bgdSize = m_bgdLabel->getSize();
                m_bgdLabel->setAngle(angle);
                m_bgdLabel->setOpacity(opacity);
            }
            else
            {
                embellishLayer.insert("frame", frame);
                embellishLayer.insert("filename", fileName);
                embellishLayer.insert("picture", ba);
                embellishLayer.insert("opacity", opacity);
                embellishLayer.insert("rotation", angle);
                embellishLayer.insert("type", type);
                m_layers << embellishLayer;
            }
        }

        if (1 == type)  // Picture layer
        {
            photoLayer.insert("frame", frame);
            photoLayer.insert("filename", fileName);
            photoLayer.insert("picture", ba);
            photoLayer.insert("maskfile", data["maskLayer"].toString() + ".png");
            photoLayer.insert("opacity", opacity);
            photoLayer.insert("rotation", angle);
            photoLayer.insert("type", type);
            photoLayers << photoLayer;
            m_layers << photoLayer;
        }

        if (3 == type)  // Mask layer
        {
            maskLayer.insert("frame", frame);
            maskLayer.insert("filename", fileName);
            maskLayer.insert("picture", ba);
            maskLayer.insert("opacity", opacity);
            maskLayer.insert("rotation", angle);
            maskLayers << maskLayer;
        }
    }

    //qDebug() << __FILE__ << __LINE__ << photoLayers;

    foreach (const QVariant &layer, photoLayers)
    {
        photoLayer = layer.toMap();
        maskFile = photoLayer["maskfile"].toString();
        for (int j = 0; j < maskLayers.size(); j++)
        {
            maskLayer = maskLayers.at(j).toMap();
            if (maskFile == maskLayer["filename"].toString())
            {
                photoLayer.insert("maskLayer", maskLayer);
                m_photoLayers << photoLayer;
                break;
            }
        }
    }

    QPoint bgdPos((ui->mainFrame->width() - bgdSize.width()) / 2, 0);
    m_bgdLabel->setRect(QRect(bgdPos, bgdSize));
}

bool EditPageWidget::swicth(int index)
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

    m_layers.clear();
    m_photoLayers.clear();
    m_bgdLabel->flush();

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        m_layerLabels[i]->flush();
    }

    ui->pagesLabel->setText(tr("第%1页/共%2页").arg(index).arg(count));

    if ((m_pAlbumWidget = static_cast<AlbumChildWidget *>(m_albumsMap[index])))
    {
        int pid = 0, tid = 0;
        AlbumPhotos photosVector = m_pAlbumWidget->getPhotosVector();

        updateLayers();

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

#if NOT_LOADED
            if (pid < m_photoLayers.size())
            //if (!pid && pid < m_photoLayers.size()) // for test
            {
                m_layerLabels[pid]->loadPhoto(//m_photoLayers.first().toMap()
                                                //m_photoLayers.last().toMap()
                                                m_photoLayers.at(pid).toMap()
                                                 ,
                                                 m_bgdLabel->getRatioSize(),
                                                 m_bgdLabel->getRect()
                                                 /*,photoFile*/);
                pid++;
            }
#endif

            ThumbChildWidget *childWidget = new ThumbChildWidget(++tid, DRAGGABLE_PHOTO, photoFile, angle, axis, m_container);
            m_pThumbsScene->insertProxyWidget(i + 1, new ThumbProxyWidget(childWidget), photoFile);
            childWidget->setId(i);

            connect(childWidget, SIGNAL(itemSelected(bool)), SLOT(selectThumb(bool)));
            connect(childWidget, SIGNAL(itemReplaced(QString,QString)), SLOT(onReplaced(QString,QString)));

            //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i << index << photoFile << photoInfo.at(3);
        }

        //qDebug() << __FILE__ << __LINE__ << pid;
        m_bgdLabel->loadLayers(pid, m_layers, m_layerLabels);
        m_bgdLabel->compose(VISIABLE_IMG_SCREEN);

        QStringList photosList = m_pAlbumWidget->getPhotosList();

        if (!m_pThumbsScene->isEmpty())
        {
            adjustThumbsHeight();
            m_pThumbsScene->adjustViewLayout();
        }

        ThumbChildWidget::updateList(photosList);

        m_pTemplatePage->changeTmplFile(m_pAlbumWidget->getTmplLabel().getBelongings());

        ok = true;
    }

    return ok;
}

void EditPageWidget::on_editPushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->editPushButton->setCheckable(true);
    m_bgdLabel->show();
    showPhotos(true);
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->hide();
    m_pTemplatePage->hide();
}

void EditPageWidget::on_photoPushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->photoPushButton->setCheckable(true);
    m_bgdLabel->hide();
    showPhotos(false);
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->show();
    m_pTemplatePage->hide();
    adjustViewLayout();
}

void EditPageWidget::on_templatePushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 0);
    ui->templatePushButton->setCheckable(true);
    m_bgdLabel->hide();
    showPhotos(false);
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

    swicth(m_current);
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

    swicth(m_current);
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

        if (!swicth(m_current))
        {
            m_container->enterEdit(false);
        }
    }
}

void EditPageWidget::on_zoomInPushButton_clicked()
{
    if (m_pAlbumWidget && m_layerLabel)
    {
        if (m_layerLabel->zoomAction(true, 1.10))
        {
            m_layerLabel->updateRect();
            m_bgdLabel->compose(VISIABLE_IMG_SCREEN);
            //qDebug() << __FILE__ << __LINE__ << m_layerLabel->getFrame();
        }
        else
        {
            ui->zoomInPushButton->setEnabled(false);
        }
    }
}

void EditPageWidget::on_zoomOutPushButton_clicked()
{
    if (m_pAlbumWidget && m_layerLabel)
    {
        if (m_layerLabel->zoomAction(false, 0.90))
        {
            m_layerLabel->updateRect();
            m_bgdLabel->compose(VISIABLE_IMG_SCREEN);
            //qDebug() << __FILE__ << __LINE__ << m_layerLabel->getFrame();
        }
        else
        {
            ui->zoomOutPushButton->setEnabled(false);
        }
    }
}

void EditPageWidget::on_resetPushButton_clicked()
{
//    if (m_pAlbumWidget)
//    {
//        m_bgdLabel->compose(VISIABLE_IMG_ORIGINAL, "C:\\Users\\Onglu\\Desktop\\test\\Composed_Final.jpg");
//    }

    if (m_pAlbumWidget && m_layerLabel)
    {
        m_layerLabel->resetAction();
        m_layerLabel->updateRect();
        m_bgdLabel->compose(VISIABLE_IMG_SCREEN);
    }
}

void EditPageWidget::on_mirroredPushButton_clicked()
{
    if (m_layerLabel)
    {
        m_layerLabel->flipAction();
        m_layerLabel->updateRect();
        m_bgdLabel->compose(VISIABLE_IMG_SCREEN);
    }
}

#if 0
void EditPageWidget::on_zoomInPushButton_pressed()
{
    if (m_layerLabel && !m_layerLabel->zoomAction(true, 1.10))
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
    if (m_layerLabel && !m_layerLabel->zoomAction(false, 0.90))
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
        m_bgdLabel->compose(VISIABLE_IMG_SCREEN);
        qDebug() << __FILE__ << __LINE__ << m_layerLabel->getFrame();
    }
}

inline void EditPageWidget::enableButtons(bool enable)
{
    ui->zoomInPushButton->setEnabled(enable);
    ui->zoomOutPushButton->setEnabled(enable);
    ui->resetPushButton->setEnabled(enable);
    ui->mirroredPushButton->setEnabled(enable);
}
