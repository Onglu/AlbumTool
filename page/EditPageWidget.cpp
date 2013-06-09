#include "EditPageWidget.h"
#include "ui_EditPageWidget.h"
#include "TaskPageWidget.h"
#include "child/ThumbChildWidget.h"
#include "child/RegulableWidget.h"
#include "wrapper/PhotoLayer.h"
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
    m_layerLabels(LayerLabelsVector(PHOTOS_NUMBER)),
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
        connect(layer, SIGNAL(clicked(PhotoLayer&,QPoint)), SLOT(labelClicked(PhotoLayer&,QPoint)));
    }
#endif

    m_pThumbsScene = new PictureGraphicsScene(Qt::gray,
                                              PictureGraphicsScene::LayoutMode_Horizontality,
                                              PictureGraphicsScene::SceneType_Thumbs,
                                              ui->thumbsGraphicsView,
                                              container);
    m_container->m_scensVector.insert(PictureGraphicsScene::SceneType_Thumbs, m_pThumbsScene);

    ui->photosGraphicsView->setScene(m_container->m_pPhotosScene);

    m_pTemplatePage = new TemplatePageWidget(true);
    m_pTemplatePage->getView()->setScene(m_container->m_pTemplatesScene);
    ui->mainHorizontalLayout->addWidget(m_pTemplatePage);

    on_editPushButton_clicked();

    connect(m_pTemplatePage, SIGNAL(replaced(QString,QString)), SLOT(onReplaced(QString,QString)));
}

EditPageWidget::~EditPageWidget()
{
    delete ui;
}

void EditPageWidget::labelClicked(PhotoLayer &label, QPoint pos)
{
    if (label.isMoveable())
    {
        m_bgdLabel->enterCopiedRect(true, label.visiableImg(), label.visiableRect(PhotoLayer::VisiableRectTypeFixed));
        m_layerLabel = &label;
        m_startPos = pos;
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_startPos << pos;
    }
}

void EditPageWidget::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && m_layerLabel && m_layerLabel != childAt(event->pos()))
    {
        m_layerLabel = NULL;
    }
}

void EditPageWidget::mouseMoveEvent(QMouseEvent *event)
{ 
    if ((event->pos() - m_startPos).manhattanLength() >= QApplication::startDragDistance()
            && m_layerLabel && m_layerLabel->isMoveable())
    {
        QPoint pos = m_layerLabel->geometry().topLeft();
        pos.rx() += event->pos().x() - m_startPos.x();
        pos.ry() += event->pos().y() - m_startPos.y();
        m_layerLabel->movePhoto(QPoint(event->pos().x() - m_startPos.x(), event->pos().y() - m_startPos.y()));
        m_startPos = event->pos();
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_startPos;
    }
}

void EditPageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && m_layerLabel && m_layerLabel->isMoveable())
    {
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_layerLabel->pos();
        m_bgdLabel->enterCopiedRect(false);
        m_layerLabel->setMoveable(false);
        m_bgdLabel->compose(m_layers, m_layerLabels);
        //m_layerLabel = NULL;
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
    if (m_bgdLabel->isVisible() && m_bgdLabel->hasPicture())
    {
        m_bgdLabel->setPixmap(QPixmap(m_bgdPic).scaled(m_bgdLabel->size(),
                                                       Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation));
    }

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

    if (PHOTOS_NUMBER == m_pThumbsScene->getProxyWidgetsMap().size() && 1072 > ui->mainFrame->width())
    {
        height = 178;
    }

    ui->thumbsGraphicsView->setMinimumHeight(height);
    ui->thumbsGraphicsView->setMaximumHeight(height);
}

#if 0
void EditPageWidget::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton & event->buttons())
    {
#if NOT_LOADED
        //for (int i = 0; i < m_photoLayers.size(); i++)
        for (int i = m_photoLayers.size() - 1; i > 0; i--)
        {
            if (m_photoWidgets[i]->hasLoaded())
            {
                QRect rect = m_photoWidgets[i]->visiableRect(VISIABLE_RECT_TYPE_CONTAINER);
                //m_photoWidgets[i]->showButtons(rect.contains(event->pos()));

                //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << event->pos() << ;

                m_photoWidgets[i]->showButtons(false);

                if (rect.x() < event->pos().x() && rect.y() < event->pos().y()
                   && rect.right() > event->pos().x() && rect.bottom() > event->pos().y())
                {
                    m_photoWidgets[i]->showButtons(true);
                    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i;
                    break;
                }
            }
        }
#endif
    }
}
#endif

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
    ProxyWidgetsMap &proxyWidgets = m_pThumbsScene->getProxyWidgetsMap();

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
    ProxyWidgetsMap &thumbWidgetsMap = m_pThumbsScene->getProxyWidgetsMap();

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

//    if (ui->bgdLayerLabel->isVisible())
//    {

//    }
//    else
    {
        if (ui->photosGraphicsView->isVisible())
        {
            m_container->replace(PictureGraphicsScene::SceneType_Photos, current, replaced);
            updateAlbum();
            m_container->noticeChanged();
        }
        else
        {
            m_container->replace(PictureGraphicsScene::SceneType_Templates, current, replaced);
            m_pAlbumWidget->changeTemplate(replaced);
        }
    }
}

void EditPageWidget::updateLayers()
{
    Q_ASSERT(m_pAlbumWidget);

    QSize bgdSize = m_pAlbumWidget->getCanvasSize();
    QString tmplDir = m_tmplPic.left(m_tmplPic.lastIndexOf(QDir::separator()) + 1);
    QString maskfile, filename;
    QVariantMap photoLayer, maskLayer;
    QVariantList photoLayers, maskLayers, &layers = m_pAlbumWidget->getLayersList();

    foreach (const QVariant &layer, layers)
    {
        qreal opacity = 1, angle = 0;
        QVariantMap embellishLayer, data = layer.toMap();
        QVariantMap frame = data["frame"].toMap();
        int width = frame["width"].toInt();
        int height = frame["height"].toInt();
        int x = frame["x"].toInt();
        int y = frame["y"].toInt();
        int type;

        filename = tmplDir + data["id"].toString() + ".png";

        if (bgdSize.width() == width && bgdSize.height() == height && x == width / 2 && y == height / 2)
        {
            QPixmap bgdPix(m_bgdPic = filename);
            if (!bgdPix.isNull())
            {
                m_bgdLabel->setRealSize(bgdSize);
                bgdSize = m_bgdLabel->loadPixmap(filename);
            }
        }

        maskfile = tmplDir + data["maskLayer"].toString() + ".png";
        opacity = data["opacity"].toReal();
        angle = data["rotation"].toReal();
        type = data["type"].toInt();

        if (0 == type && m_bgdPic != filename)
        {
            if (m_bgdPic == filename)
            {
                m_bgdLabel->setAngle(angle);
                m_bgdLabel->setOpacity(opacity);
            }
            else
            {
                embellishLayer.insert("frame", frame);
                embellishLayer.insert("filename", filename);
                embellishLayer.insert("opacity", opacity);
                embellishLayer.insert("rotation", angle);
                photoLayer.insert("type", type);
                m_layers << embellishLayer;
            }
        }

        if (1 == type)
        {
            photoLayer.insert("frame", frame);
            photoLayer.insert("filename", filename);
            photoLayer.insert("maskfile", maskfile);
            photoLayer.insert("opacity", opacity);
            photoLayer.insert("rotation", angle);
            photoLayer.insert("type", type);
            photoLayers << photoLayer;
            m_layers << photoLayer;
        }

        if (3 == type)
        {
            maskLayer.insert("frame", frame);
            maskLayer.insert("filename", filename);
            maskLayer.insert("opacity", opacity);
            maskLayer.insert("rotation", angle);
            maskLayers << maskLayer;
        }
    }

    //qDebug() << __FILE__ << __LINE__ << photoLayers << maskLayers;

    foreach (const QVariant &layer, photoLayers)
    {
        photoLayer = layer.toMap();
        maskfile = photoLayer["maskfile"].toString();
        for (int j = 0; j < maskLayers.size(); j++)
        {
            maskLayer = maskLayers.at(j).toMap();
            if (maskfile == maskLayer["filename"].toString())
            {
                photoLayer.insert("maskLayer", maskLayer);
                m_photoLayers << photoLayer;
                break;
            }
        }
    }

    //QPoint bgdPos = ui->bgdLayerLabel->mapTo(this, QPoint(0, 0));
    //QPoint bgdPos = ui->mainFrame->pos();
    //bgdPos.rx() += (ui->mainFrame->width() - bgdSize.width()) / 2;
    //bgdPos.ry() = 0;
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
    m_pThumbsScene->clearProxyWidgetsMap();

    m_bgdPic.clear();
    m_photoLayers.clear();
    m_bgdLabel->flush();

    ui->pagesLabel->setText(tr("第%1页/共%2页").arg(index).arg(count));

    if ((m_pAlbumWidget = static_cast<AlbumChildWidget *>(m_albumsMap[index])))
    {
        int index = 0;
        AlbumPhotos photosVector;

        m_pAlbumWidget->getViewsList(photosVector, m_tmplPic);
        updateLayers();

        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QStringList photoInfo = photosVector[i].split(TEXT_SEP);
            if (4 != photoInfo.size())
            {
                continue;
            }

            QString photoFile = photoInfo.at(0);
            qreal angle = photoInfo.at(1).toDouble();
            Qt::Axis axis = (Qt::Axis)photoInfo.at(2).toInt();

#if NOT_LOADED
//            bool loaded = false;

//            for (int i = 0; i < m_photoLayers.size(); i++)
//            {
//                if (m_photoWidgets[i]->hasLoaded(photoFile))
//                {
//                    loaded = true;
//                    break;
//                }
//            }

            if (/*!loaded && */index < m_photoLayers.size())
            {
                QImage img = m_layerLabels[index]->loadPhoto(//m_photoLayers.first().toMap()
                                                  //m_photoLayers.last().toMap()
                                                 m_photoLayers.at(index).toMap()
                                                 ,
                                                 m_bgdLabel->getRatioSize(),
                                                 m_bgdLabel->getRect()
                                                 /*,photoFile*/);

                //m_layerLabels[index]->hide();
                //qDebug() << __FILE__ << __LINE__ << index << m_layerLabels[index]->visiableRect(VISIABLE_RECT_TYPE_CONTAINER);
                //ui->bgdLayerLabel->setPixmap(QPixmap::fromImage(img));
                //m_layerLabels[index]->setPixmap(QPixmap::fromImage(img));
            }
#endif

            ThumbChildWidget *childWidget = new ThumbChildWidget(++index, DRAGGABLE_PHOTO, photoFile, angle, axis, m_container);
            m_pThumbsScene->insertProxyWidget(i + 1, new ThumbProxyWidget(childWidget), photoFile);
            childWidget->setId(i);

            connect(childWidget, SIGNAL(itemSelected(bool)), SLOT(selectThumb(bool)));
            connect(childWidget, SIGNAL(itemReplaced(QString,QString)), SLOT(onReplaced(QString,QString)));

            //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i << index << photoFile << photoInfo.at(3);
        }

        m_bgdLabel->compose(m_layers, m_layerLabels);

        QStringList photosList = m_pAlbumWidget->getPhotosList();

        if (!m_pThumbsScene->isEmpty())
        {
            adjustThumbsHeight();
            m_pThumbsScene->adjustViewLayout();
        }

        ThumbChildWidget::updateList(photosList);

        m_pTemplatePage->setPreview(m_tmplPic);

        ok = true;
    }

    return ok;
}

void EditPageWidget::on_editPushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    //ui->mainHorizontalLayout->setContentsMargins(0, TOP_FRAME_HEIGHT, 0, TOP_FRAME_HEIGHT);
    ui->editPushButton->setCheckable(true);
    m_bgdLabel->show();
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->hide();
    m_pTemplatePage->hide();
}

void EditPageWidget::on_photoPushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 9);
    ui->photoPushButton->setCheckable(true);
    m_bgdLabel->hide();
    ui->thumbsGraphicsView->show();
    ui->photosGraphicsView->show();
    m_pTemplatePage->hide();
    adjustViewLayout();
}

void EditPageWidget::on_templatePushButton_clicked()
{
    ui->verticalLayout->setContentsMargins(9, 9, 9, 0);
    //ui->mainHorizontalLayout->setMargin(0);
    ui->templatePushButton->setCheckable(true);
    m_bgdLabel->hide();
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

void EditPageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_PHOTO) && !children().contains(event->source()))
    {
        event->acceptProposedAction();
    }
}

void EditPageWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_PHOTO) && !children().contains(event->source()))
    {
        event->acceptProposedAction();
    }
}

void EditPageWidget::dropEvent(QDropEvent *event)
{
    DraggableLabel *picLabel = static_cast<DraggableLabel *>(event->source());
    if (picLabel->meetDragDrop(DRAGGABLE_PHOTO) && !children().contains(picLabel))
    {
        QVariantMap belongings = picLabel->getBelongings();
        //qDebug() << __FILE__ << __LINE__ << belongings << event->pos();

        //for (int i = 0; i < m_photoLayers.size(); i++)
        for (int i = m_photoLayers.size(); i > 0; i--)
        {
//            if (m_photoWidgets[i]->hasLoaded())
//            {
//                QRect rect = m_photoWidgets[i]->visiableRect(VISIABLE_RECT_TYPE_CONTAINER);
//                //m_photoWidgets[i]->showButtons(rect.contains(event->pos()));

//                //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << event->pos() << ;

//                if (rect.x() < event->pos().x() && rect.y() < event->pos().y()
//                   && rect.right() > event->pos().x() && rect.bottom() > event->pos().y())
//                {
//                    m_photoWidgets[i]->changePicture(belongings);
//                    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i << m_photoWidgets[i]->photoFile();
//                    event->acceptProposedAction();
//                    break;
//                }

////                if (rect.contains(event->pos()))
////                {
////                    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << i << m_photoWidgets[i]->photoFile();
////                }
//            }
        }
    }
}

void EditPageWidget::on_mirroredPushButton_clicked()
{
    if (m_layerLabel)
    {
        m_layerLabel->flipAction();
        m_layerLabel->updateRect();
        m_bgdLabel->compose(m_layers, m_layerLabels);
    }
}
