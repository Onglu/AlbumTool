#include "AlbumChildWidget.h"
#include "ui_AlbumChildWidget.h"
#include "page/TaskPageWidget.h"
#include "wrapper/utility.h"
#include <QDebug>

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
                                   TaskPageWidget *parent) :
    PictureChildWidget(QSize(118, 190), true, parent),
    ui(new Ui::AlbumChildWidget),
    m_photosVector(photosList.toVector()),
    m_tmplVisible(false),
    m_locked(false)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    memset(m_locations, 0, 2);

    QString tmplPic;
    setupWidgets(photosList, readTmplData(tmplFile, tmplPic));

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

const QString &AlbumChildWidget::readTmplData(const QString &tmplFile, QString &tmplPic)
{
    QVariantMap canvas;
    FileParser fp(tmplFile);

    if (tmplFile.isEmpty() || m_tmplFile == tmplFile || !fp.openTemplate(m_bases, canvas, m_tags, m_layers))
    {
        return tmplPic;
    }

    m_tmplFile = tmplFile;

    m_canvasSize = QSize(canvas["width"].toInt(), canvas["height"].toInt());

    QString tmplDir = tmplFile.left(tmplFile.lastIndexOf(QDir::separator()) + 1);
    tmplPic = QString("%1%2.psd.png").arg(tmplDir).arg(/*iter.value()*/ m_bases["name"].toString());

    m_locations[0] = (uchar)m_bases["landscapeCount"].toUInt();
    m_locations[1] = (uchar)m_bases["portraitCount"].toUInt();

    //qDebug() << __FILE__ << __LINE__ << m_locations[0] + m_locations[1];

//    qDebug() << __FILE__ << __LINE__ << "size:" << size["width"] << "x" << size["height"];

//    foreach (const QVariant &tag, tags)
//    {
//        attr = tag.toMap();
//        qDebug() << __FILE__ << __LINE__ << "tag:" << attr["name"] << "," << attr["type"];
//    }

//    foreach (const QVariant &layer, layers)
//    {
//        attr = layer.toMap();
//        QVariantMap::const_iterator nested = attr.constBegin();
//        while (nested != attr.constEnd())
//        {
//            if ("frame" == nested.key())
//            {
//                QVariantMap frame = nested.value().toMap();
//                qDebug() << __FILE__ << __LINE__ << "frame:" << frame["height"] << "," << frame["width"] << "," << frame["x"]<< "," << frame["y"];
//            }
//            else
//            {
//                qDebug() << __FILE__ << __LINE__ << "frame:" << nested.key() << "=" << nested.value();
//            }

//            ++nested;
//        }
//    }

    return tmplPic;
}

void AlbumChildWidget::setupWidgets(const QStringList &photosList, const QString &tmplPic)
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

    if (!tmplPic.isEmpty())
    {
        /* Initialize the template that is at the back of photos list */
        pix = QPixmap(tmplPic).scaled(QSize(92, 142), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_tmplLabel->setContentsMargins(1, 1, 2, 2);
        m_tmplLabel->setPixmap(pix);
    }

    TemplateChildWidget::setTemplate(*m_tmplLabel, tmplPic, m_tmplFile);
    //qDebug() << __FILE__ << __LINE__ << m_tmplLabel->getBelongings();

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

void AlbumChildWidget::changeTemplate(const QString &tmplFile, const QPixmap &tmplPic, const QVariantMap &belongings)
{
    if (m_tmplFile == tmplFile)
    {
        return;
    }

    if (tmplFile.isEmpty())
    {
        m_tmplFile.clear();
        m_tmplLabel->reset();
        memset(m_locations, 0, 2);
        changeBanners();
    }
    else
    {
#if 0
        QString tmplPic;
        QPixmap pix(readTmplData(tmplFile, tmplPic));

        m_tmplLabel->setPixmap(pix.scaled(QSize(93, 141), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_tmplLabel->clearMimeType();

        if (data.isEmpty())
        {
            TemplateChildWidget::setTemplate(*m_tmplLabel, tmplPic, tmplFile);
        }
        else
        {
            m_tmplLabel->setBelongings(data);
        }

        changeBanners();
        m_container->noticeChanged();
#endif


        m_tmplLabel->setPixmap(tmplPic.scaled(QSize(93, 141), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_tmplLabel->clearMimeType();
        m_tmplLabel->setBelongings(belongings);
    }
}

void AlbumChildWidget::open(ChildWidgetsMap &widgetsMap)
{
    TaskPageWidget *container = static_cast<TaskPageWidget *>(m_container);
    if (container)
    {
        container->onEdit(widgetsMap, m_index);
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

        //qDebug() << __FILE__ << __LINE__ << picFile;

        m_tmplVisible = m_tmplLabel != m_picLabel ? false : true;

        if (!m_tmplVisible)
        {
            QString picFile = belongings["picture_file"].toString();
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
            QString tmplFile = belongings["template_file"].toString();
            belongings = m_tmplLabel->getBelongings();
            if (tmplFile == belongings["template_file"].toString())
            {
                return;
            }

            if (m_picLabel->hasPicture())
            {
                m_container->replace(PictureGraphicsScene::SceneType_Templates,
                                     m_picLabel->getPictureFile(),
                                     tmplFile);
            }
            else
            {
                picLabel->accept();
            }

            //changeTemplate(tmplFile, picLabel->getBelongings());
            changeTemplate(tmplFile, picLabel->getPicture(false), /*belongings["page_data"].toMap()*/ belongings);
            //qDebug() << __FILE__ << __LINE__ << m_tmplLabel->getBelongings();
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
    if (!readTmplData(tmplFile, tmplPic).isEmpty())
    {
        QPixmap pix(tmplPic);
        if (!pix.isNull())
        {
            m_tmplLabel->setPixmap(pix.scaled(QSize(93, 141), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        TemplateChildWidget::setTemplate(*m_tmplLabel, tmplPic, tmplFile);
    }

    changeBanners();
}

void AlbumChildWidget::getViewsList(AlbumPhotos &photosVector, QString &tmpl, bool pic)
{
    photosVector = m_photosVector;
    tmpl = pic ? m_tmplLabel->getPictureFile() : m_tmplFile;
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
        m_pChildWidget->changeTemplate();// clear template
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

        AlbumPhotos photosVector(PHOTOS_NUMBER);
        DraggableLabels &photoLabels = m_pChildWidget->getPhotoLabels();
        QString tmplPic;

        m_pChildWidget->getViewsList(photosVector, tmplPic);

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
