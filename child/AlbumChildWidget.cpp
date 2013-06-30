#include "AlbumChildWidget.h"
#include "ui_AlbumChildWidget.h"
#include "page/TaskPageWidget.h"
#include "wrapper/utility.h"
#include "parser/json.h"
#include "defines.h"
#include <QDebug>

using namespace QtJson;

AlbumChildWidget::AlbumChildWidget(int index, TaskPageWidget *parent) :
    PictureChildWidget(QSize(118, 190), true, parent),
    ui(new Ui::AlbumChildWidget),
    m_photosVector(AlbumPhotos(PHOTOS_NUMBER)),
    m_tmplVisible(false),
    m_locked(false)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    memset(m_locations, 0, 2);

    setupWidgets();
}

AlbumChildWidget::AlbumChildWidget(int index,
                                   const QStringList &photosList,
                                   const QString &tmplFile,
                                   const QVariantList &photoLayers,
                                   TaskPageWidget *parent) :
    PictureChildWidget(QSize(118, 190), true, parent),
    ui(new Ui::AlbumChildWidget),
    m_photosVector(photosList.toVector()),
    m_tmplFile(tmplFile),
    m_tmplVisible(false),
    m_locked(false)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    memset(m_locations, 0, 2);

    setupWidgets(photosList, tmplFile, photoLayers);

    int photosNum = m_photosList.size();
    if (photosNum /*|| locations*/)
    {
        //ui->usageLabel->setText(tr("%1/%2").arg(photosNum).arg(locations));
        //ui->usageLabel->setToolTip(tr("照片数量：%1张，模板相位：%2个").arg(photosNum).arg(locations));
    }
}

AlbumChildWidget::~AlbumChildWidget()
{
    delete ui;
}

void AlbumChildWidget::setupWidgets(const QStringList &photosList,
                                    const QString &tmplFile,
                                    const QVariantList &photoLayers)
{
    QPixmap pix;
    QSize photoSize(48, 48), tmplSize(96, 144);

    m_photoLabels.append(new DraggableLabel(photoSize, DRAGGABLE_PHOTO, ui->photoLabel1));
    m_photoLabels.append(new DraggableLabel(photoSize, DRAGGABLE_PHOTO, ui->photoLabel2));
    m_photoLabels.append(new DraggableLabel(photoSize, DRAGGABLE_PHOTO, ui->photoLabel3));
    m_photoLabels.append(new DraggableLabel(photoSize, DRAGGABLE_PHOTO, ui->photoLabel4));
    m_photoLabels.append(new DraggableLabel(photoSize, DRAGGABLE_PHOTO, ui->photoLabel5));
    m_photoLabels.append(new DraggableLabel(photoSize, DRAGGABLE_PHOTO, ui->photoLabel6));

    m_photosList.clear();

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        /* Initialize the persent album with the photos list */
        QString photoAttr;

        if (!photosList.isEmpty() && "" != (photoAttr = photosList.at(i)))
        {
            QStringList photoInfo = photoAttr.split(TEXT_SEP);
            if (4 == photoInfo.size())
            {
                QString photoFile = photoInfo.at(0);
                qreal angle = photoInfo.at(1).toDouble();
                Qt::Axis axis = (Qt::Axis)photoInfo.at(2).toInt();
                int usedTimes = photoInfo.at(3).toInt();

                if (!QFile::exists(photoFile))
                {
                    m_photosVector[i].clear();
                }
                else
                {
                    if (!m_photosList.contains(photoFile, Qt::CaseInsensitive))
                    {
                        m_photosList.append(photoFile);

                        pix = QPixmap(photoFile).scaled(QSize(46, 46), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        if (angle || Qt::ZAxis != axis)
                        {
                            pix = pix.transformed(QTransform().rotate(angle, axis));
                        }

                        m_photoLabels.at(i)->setPixmap(pix);
                        PhotoChildWidget::setPhoto(*m_photoLabels.at(i), photoFile, angle, axis, usedTimes);
                    }
                }
            }
        }

        m_photoLabels.at(i)->setContentsMargins(1, 1, 2, 2);
        m_photoLabels.at(i)->installEventFilter(this);
    }

    m_picLabel = m_photoLabels.last();
    m_tmplLabel = new DraggableLabel(tmplSize, DRAGGABLE_TEMPLATE, ui->templateLabel);

    QString tmplPic;
    TemplateChildWidget tmplWidget(tmplFile, m_tmplLabel);

    if (tmplWidget.getTmplPic(tmplPic))
    {
        m_tmplLabel->setContentsMargins(1, 1, 2, 2);
        m_tmplLabel->setPicture(QPixmap(tmplPic), QSize(92, 142));
        m_records.insert("photo_layers", photoLayers);
    }

    //qDebug() << __FILE__ << __LINE__ << m_tmplFile << tmplPic;

    m_tmplLabel->setVisible(false);
    m_tmplLabel->installEventFilter(this);

    ui->innerHorizontalLayout->setAlignment(Qt::AlignLeft);
    changeBanners();

    m_records.insert("photos_list", photosList);
    m_records.insert("template_file", m_tmplFile);
}

inline void AlbumChildWidget::changeBanners()
{
    int usages = m_photosList.size();
    int num = m_locations[0] + m_locations[1] - usages;

    clearBanners();

    if (0 < num)
    {
        addBanners(usages, ":/images/usages_num.png", "已用图片位");
        addBanners(num, ":/images/available_num.png", "可用图片位");
    }
    else if (0 == num || 0 == m_locations[0] + m_locations[1])
    {
        addBanners(usages, ":/images/usages_num.png", "已用图片位");
    }
    else
    {
        addBanners(usages + num, ":/images/usages_num.png", "可用图片位");
        addBanners(-1 * num, ":/images/over_num.png", "超出图片位");
    }

    //qDebug() << __FILE__ << __LINE__ << usages << num;
}

inline void AlbumChildWidget::addBanner(QString banner, QString tip)
{
    QLabel *lable = new QLabel;
    lable->setFixedSize(12, 12);
    lable->setPixmap(QPixmap(banner));
    lable->setToolTip(tip);
    ui->innerHorizontalLayout->addWidget(lable);
}

void AlbumChildWidget::addBanners(int count, QString banner, QString tip)
{
    for (int i = 0; i < count; i++)
    {
        addBanner(banner, tip);
    }
}

void AlbumChildWidget::clearBanners()
{
    QLayoutItem *item;
    while ((item = ui->innerHorizontalLayout->itemAt(0)))
    {
        QWidget *widget = item->widget();
        ui->innerHorizontalLayout->removeWidget(widget);
        ui->innerHorizontalLayout->removeItem(item);
        delete widget;
        widget = NULL;
    }
}

void AlbumChildWidget::changeTmplFile(const QVariantMap &belongings)
{
    m_records["photo_layers"].clear();

    if (belongings.isEmpty())
    {
        m_tmplFile.clear();
        m_tmplLabel->reset();
        memset(m_locations, 0, 2);
        changeBanners();
    }
    else
    {
        QPixmap tmplPic(belongings["picture_file"].toString());
        if (!tmplPic.isNull())
        {
            tmplPic = tmplPic.scaled(QSize(93, 141), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        m_tmplLabel->setPixmap(tmplPic);
        m_tmplLabel->clearMimeType();
        m_tmplLabel->setBelongings(belongings);
        m_tmplFile = belongings["template_file"].toString();

        TemplateChildWidget::getLocations(belongings["page_data"].toMap(), m_locations);
        //qDebug() << __FILE__ << __LINE__ << m_locations[0] << m_locations[1];
        changeBanners();
        m_container->noticeChanged();
    }
}

void AlbumChildWidget::changePhotoLayer(const QString &photoName,
                                        QRect rect,
                                        qreal opacity,
                                        qreal angle)
{
    QVariantMap &belongings = m_tmplLabel->getBelongings();
    QVariantMap page = belongings["page_data"].toMap();
    QVariantList info, layers = page["layers"].toList(), changes = m_records["photo_layers"].toList();
    bool changed = false;

    //qDebug() << __FILE__ << __LINE__ << "before:" << layers;
    //qDebug() << __FILE__ << __LINE__ << "before:" << photoName << rect;

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        QString id = data["id"].toString();
        if (photoName == id)
        {
            QVariantMap frame;
            frame.insert("width", rect.width());
            frame.insert("height", rect.height());
            frame.insert("x", rect.x());
            frame.insert("y", rect.y());
            data.insert("frame", frame);
            data.insert("opacity", opacity);
            data.insert("rotation", angle);
            data.insert("orientation", rect.width() > rect.height() ? 1 : 0);

            foreach (const QVariant &change, changes)
            {
                QVariantMap related = change.toMap();
                if (photoName == related["id"].toString())
                {
                    changes.removeOne(change);
                    break;
                }
            }

            if (!changed)
            {
                changed = true;
            }

            changes << data;
            //qDebug() << __FILE__ << __LINE__ << "found";
        }

        info << data;
    }

    page.insert("layers", info);
    belongings.insert("page_data", page);
    //qDebug() << __FILE__ << __LINE__ << "after:" << changes;

    if (changed)
    {
        m_records.insert("photo_layers", changes);
        m_container->noticeChanged();
    }
}

QSize AlbumChildWidget::getSize()
{
    QVariantMap belongings = m_tmplLabel->getBelongings();
    return TemplateChildWidget::getSize(belongings["page_data"].toMap());
}

const QVariantList &AlbumChildWidget::getLayers()
{
    QVariantMap &belongings = m_tmplLabel->getBelongings();
    QVariantMap page = belongings["page_data"].toMap();
    QVariantList info, layers = page["layers"].toList(), changes = m_records["photo_layers"].toList();

    //qDebug() << __FILE__ << __LINE__ << "before:" << changes.size() << changes;

    if (changes.isEmpty())
    {
        return layers;
    }

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        QString photoName = data["id"].toString();
        foreach (const QVariant &change, changes)
        {
            QVariantMap related = change.toMap();
            if (photoName == related["id"].toString())
            {
                data = related;
                changes.removeOne(change);
                break;
            }
        }

        info << data;
    }

    page.insert("layers", info);
    belongings.insert("page_data", page);

//    QFile jf("C:\\Users\\Onglu\\Desktop\\test\\changes.json");
//    if (jf.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        QByteArray result = QtJson::serialize(page);
//        jf.write(result);
//        jf.close();
//    }

    //qDebug() << __FILE__ << __LINE__ << "after:" << page;

    return info;
}

void AlbumChildWidget::open(ChildWidgetsMap &widgetsMap)
{
    //TaskPageWidget *container = static_cast<TaskPageWidget *>(m_container);
    if (m_container)
    {
        m_container->onEdit(widgetsMap, m_index);
    }
}

inline void AlbumChildWidget::showPhotosView(bool visible)
{
    if (visible != ui->photoLabel1->isVisible())
    {
        ui->photoLabel1->setVisible(visible);
        ui->photoLabel2->setVisible(visible);
        ui->photoLabel3->setVisible(visible);
        ui->photoLabel4->setVisible(visible);
        ui->photoLabel5->setVisible(visible);
        ui->photoLabel6->setVisible(visible);
        m_tmplLabel->setVisible(!visible);
    }
}

bool AlbumChildWidget::meetDragDrop(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_PHOTO))
    {
        showPhotosView(true);  
        DraggableLabel *photoLabel = static_cast<DraggableLabel *>(childAt(event->pos()));
        if (photoLabel && photoLabel->objectName().startsWith("photoLabel"))
        {
            bool droppable = false;

            if (!photoLabel->hasPicture())
            {
                droppable = true;
            }
            else
            {
                for (int i = 0; i < PHOTOS_NUMBER; i++)
                {
                    if (!m_photoLabels.at(i)->pixmap())
                    {
                        droppable = true;
                        photoLabel = m_photoLabels.at(i);
                    }
                }
            }

            if (droppable)
            {
                m_thumbSize = QSize(48, 48);
                m_picLabel = photoLabel;
                return true;
            }
        }
    }
    else if (event->mimeData()->hasFormat(DRAGGABLE_TEMPLATE))
    {
        showPhotosView(false);
        m_thumbSize = QSize(96, 144);
        m_picLabel = m_tmplLabel;
        return true;
    }

    m_picLabel = NULL;
    event->ignore();

    return false;
}

void AlbumChildWidget::dragEnterEvent(QDragEnterEvent *event)
{
    PictureChildWidget::dragEnterEvent(event);
    if (m_dragging)
    {
        m_locked = true;
    }
}

void AlbumChildWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    if (m_locked)
    {
        bool tmplVisible = ui->templateLabel->isVisible();

        /***
         * show or hide tempalte based on its state before and after:
         * case 1: tempalte is visiable at persent, but it is invisiable at previous;
         * case 2: tempalte is invisiable at persent, but it is visiable at previous.
         */
        if (m_tmplVisible != tmplVisible && !m_dropped)
        {
            showPhotosView(!m_tmplVisible); // restore the visiable state of the tempalte
        }

        m_locked = m_dragging = false;
    }
}

void AlbumChildWidget::dropEvent(QDropEvent *event)
{
    DraggableLabel *picLabel = static_cast<DraggableLabel *>(event->source());
    if (picLabel && picLabel->hasPicture() && meetDragDrop(event))
    {
        QVariantMap belongings = picLabel->getBelongings();
        QString picFile = belongings["picture_file"].toString();
        //qDebug() << __FILE__ << __LINE__ << picFile << picLabel->getPictureFile();

        m_tmplVisible = m_tmplLabel != m_picLabel ? false : true;

        if (!m_tmplVisible)
        {
            QPixmap pix(picFile);
            if (pix.isNull() || m_photosList.contains(picFile, Qt::CaseInsensitive))
            {
                return;
            }

            picLabel->accept();

            qreal angle = belongings["rotation_angle"].toReal();
            Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
            int usedTimes = belongings["used_times"].toInt();

            pix = pix.scaled(m_thumbSize - QSize(2, 2), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_picLabel->setPixmap(pix.transformed(QTransform().rotate(angle, axis)));
            m_picLabel->setBelongings(belongings);
            m_picLabel->clearMimeType();

            m_photosList.append(picFile);
            if (0 == m_locations[0] + m_locations[1])
            {
                addBanner(":/images/usages_num.png", "已用图片位");
            }
            else
            {
                changeBanners();
            }

            int index = m_picLabel->objectName().right(1).toInt() - 1;
            m_photosVector[index] = QString("%1|%2|%3|%4").arg(picFile).arg(angle).arg(axis).arg(usedTimes);
            //qDebug() << __FILE__ << __LINE__ << m_photosVector[index];

            m_container->noticeChanged();
        }
        else
        {
            if (picFile == m_tmplLabel->getPictureFile())
            {
                return;
            }

            if (m_picLabel->hasPicture())
            {
                m_container->replace(PictureGraphicsScene::SceneType_Templates, m_picLabel->getPictureFile(), picFile);
            }
            else
            {
                picLabel->accept();
            }

            changeTmplFile(belongings);
            //qDebug() << __FILE__ << __LINE__ << tmplFile;
        }

        showPhotosView(!m_tmplVisible);

        if (children().contains(picLabel))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }

        m_dropped = true;
    }
}

bool AlbumChildWidget::eventFilter(QObject *watched, QEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent)
    {
        QLabel *childLabel = static_cast<QLabel *>(childAt(mouseEvent->pos()));
        if (watched == childLabel)
        {
            if (QEvent::MouseButtonPress == event->type())
            {
                return clickPicture();
            }
            else if (QEvent::MouseButtonDblClick == event->type())
            {
                return dblClickPicture();
            }
        }
    }

    return PictureChildWidget::eventFilter(watched, event);
}

void AlbumChildWidget::switchView(bool enter)
{
    bool tmplVisible;

    if (!ui->photoLabel1->isVisible() && ui->templateLabel->isVisible())
    {
        tmplVisible = true;
    }
    else
    {
        tmplVisible = false;
    }

    if (enter)
    {
        /* show the view layer behind of the persent view */
        if (!m_locked)
        {
            showPhotosView(tmplVisible);
        }
    }
    else
    {
        /* restore the view layer before entering */
        if (!m_locked)
        {
            showPhotosView(tmplVisible);
        }
        else
        {
            m_locked = false;
        }
    }
}

const QVariantMap &AlbumChildWidget::getChanges()
{
    QStringList photosList;

    Converter::v2s(m_photosVector, photosList);
    m_records.insert("photos_list", photosList);
    m_records.insert("template_file", m_tmplFile);

    return PictureChildWidget::getChanges();
}

void AlbumChildWidget::setViewsList(const AlbumPhotos &photosVector, const QString &tmplFile)
{
    m_photosVector = photosVector;

    QString tmplPic;
//    if (!readTmplData(tmplFile, tmplPic).isEmpty())
//    {
//        QPixmap pix(tmplPic);
//        if (!pix.isNull())
//        {
//            m_tmplLabel->setPixmap(pix.scaled(QSize(93, 141), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//        }

//        TemplateChildWidget::setTemplate(*m_tmplLabel, tmplPic, tmplFile);
//    }

    if (!tmplFile.isEmpty())
    {

    }

    changeBanners();
}

void AlbumProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    PictureProxyWidget::hoverEnterEvent(event);
    m_pChildWidget->switchView(true);
}

void AlbumProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    PictureProxyWidget::hoverLeaveEvent(event);
    m_pChildWidget->switchView(false);
}

bool AlbumProxyWidget::excludeItem(const QString &picFile)
{
    bool ok = false;

    /* Remove one picture(maybe it is a photo or template type) file from the specified album */
    if (picFile == m_pChildWidget->getTmplLabel().getPictureFile())
    {
        ok = true;
        m_pChildWidget->changeTmplFile();
    }
    else
    {
        QStringList &photosList = m_pChildWidget->getPhotosList();
        for (int i = 0; i < photosList.size(); i++)
        {
            if (picFile == photosList.at(i))
            {
                photosList.removeAt(i);
                break;
            }
        }

        AlbumPhotos photosVector = m_pChildWidget->getPhotosVector();
        DraggableLabels &photoLabels = m_pChildWidget->getPhotoLabels();

        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            if (picFile == photoLabels.at(i)->getPictureFile())
            {
                ok = true;
                photoLabels.at(i)->reset();
                photosVector[i].clear();
                break;
            }
        }

        m_pChildWidget->setViewsList(photosVector);
    }

    if (!Converter::num(m_pChildWidget->getPhotosList(), true) &&
        !m_pChildWidget->getTmplLabel().hasPicture())
    {
        m_empty = true;
        m_pChildWidget->clearBanners();
    }
    else
    {
        m_empty = false;
    }

    return ok;
}
