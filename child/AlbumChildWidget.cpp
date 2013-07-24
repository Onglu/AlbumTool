#include "AlbumChildWidget.h"
#include "ui_AlbumChildWidget.h"
#include "PhotoChildWidget.h"
#include "TemplateChildWidget.h"
#include "page/TaskPageWidget.h"
#include "page/AlbumPageWidget.h"
#include "wrapper/utility.h"
#include "parser/json.h"
#include <QDebug>
#include <QMessageBox>

using namespace QtJson;

AlbumChildWidget::AlbumChildWidget(int index, TaskPageWidget *parent) :
    PictureChildWidget(tr("第 %1 页（%2）").arg(index).arg(1 == index ? tr("封面") : tr("内页")), QSize(118, 190), true, parent),
    ui(new Ui::AlbumChildWidget),
    m_photosVector(AlbumPhotos(PHOTOS_NUMBER)),
    m_tmplVisible(false),
    m_locked(false),
    m_tmpl(NULL)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    setupWidgets();
}

AlbumChildWidget::AlbumChildWidget(int index,
                                   const QStringList &photosList,
                                   const QString &tmplFile,
                                   const QVariantList &photoLayers,
                                   TaskPageWidget *parent) :
    PictureChildWidget(tr("第 %1 页（%2）").arg(index).arg(1 == index ? tr("封面") : tr("内页")), QSize(118, 190), true, parent),
    ui(new Ui::AlbumChildWidget),
    m_photosVector(photosList.toVector()),
    m_tmplFile(tmplFile),
    m_tmplVisible(false),
    m_locked(false),
    m_tmpl(NULL)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

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
    QSize photoSize(48, 48), tmplSize(96, 144);

    memset(m_locations, 0, 2);

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
            if (PHOTO_ATTRIBUTES == photoInfo.size())
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
                        m_photoLabels[i]->loadPicture(QPixmap(photoFile), QSize(46, 46), angle, axis);
                        PhotoChildWidget::setPhoto(*m_photoLabels[i], photoFile, angle, axis, usedTimes);
                    }
                }
            }
        }

        m_photoLabels[i]->setContentsMargins(1, 1, 2, 2);
        m_photoLabels[i]->installEventFilter(this);
    }

    m_picLabel = m_photoLabels.last();
    m_tmplLabel = new DraggableLabel(tmplSize, DRAGGABLE_TEMPLATE, ui->templateLabel);

    QString tmplPic;
    TemplateChildWidget tmplWidget(tmplFile, m_tmplLabel);

    if (tmplWidget.getTmplPic(tmplPic))
    {
        m_tmplLabel->setContentsMargins(1, 1, 2, 2);
        m_tmplLabel->loadPicture(QPixmap(tmplPic), QSize(92, 142));
        QVariantMap belongings = m_tmplLabel->getBelongings();
        TemplateChildWidget::getLocations(belongings["page_data"].toMap(), m_locations);
    }

    //qDebug() << __FILE__ << __LINE__ << m_tmplFile << tmplPic;

    m_tmplLabel->setVisible(false);
    m_tmplLabel->installEventFilter(this);

    ui->innerHorizontalLayout->setAlignment(Qt::AlignLeft);
    changeBanners();

    m_records.insert("photos_list", photosList);
    m_records.insert("template_file", m_tmplFile);
    m_records.insert("photo_layers", photoLayers);
}

inline void AlbumChildWidget::changeBanners()
{
    int usages = m_photosList.size();
    int num = m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE] - usages;

    clearBanners();

    if (0 < num)
    {
        addBanners(usages, ":/images/usages_num.png", tr("已用图片位"));
        addBanners(num, ":/images/available_num.png", tr("可用图片位"));
    }
    else if (0 == num || 0 == m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE])
    {
        addBanners(usages, ":/images/usages_num.png", tr("已用图片位"));
    }
    else
    {
        addBanners(usages + num, ":/images/usages_num.png", tr("可用图片位"));
        addBanners(-1 * num, ":/images/over_num.png", tr("超出图片位"));
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

void AlbumChildWidget::changeTemplate(const QVariantMap &belongings)
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
        m_tmpl = m_container->getTmplWidget(m_tmplFile);

        TemplateChildWidget::getLocations(belongings["page_data"].toMap(), m_locations);
        //qDebug() << __FILE__ << __LINE__ << m_locations[0] << m_locations[1];
        changeBanners();
        m_container->noticeChanged();
    }
}

void AlbumChildWidget::changePhoto(const QString &fileName,
                                   QRect rect,
                                   qreal opacity,
                                   qreal angle)
{
    QVariantMap &belongings = m_tmplLabel->getBelongings();
    QVariantMap page = belongings["page_data"].toMap();
    QVariantList info, layers = page["layers"].toList(), changes = m_records["photo_layers"].toList();
    bool changed = false;

    qDebug() << __FILE__ << __LINE__ << "before:" << /*layers*/opacity;
    //qDebug() << __FILE__ << __LINE__ << "before:" << fileName << rect;

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        QString id = data["id"].toString();
        if (fileName == id)
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
                if (fileName == related["id"].toString())
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

void AlbumChildWidget::removePhoto(const QString &fileName)
{
    if (!m_photosList.contains(fileName, Qt::CaseInsensitive))
    {
        return;
    }

    bool found = false;

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        QStringList photoInfo = m_photosVector[i].split(TEXT_SEP);
        //qDebug() << __FILE__ << __LINE__ << i << photoInfo;
        if (PHOTO_ATTRIBUTES == photoInfo.size() && fileName == photoInfo.at(0))
        {
            found = true;
            m_photosList.removeOne(fileName);
            m_photosVector[i].clear();
            m_photoLabels[i]->reset();
            break;
        }
    }

    if (found)
    {
        changeBanners();
    }
}

TemplateChildWidget *AlbumChildWidget::getTmplWidget() const
{
    return m_tmpl ? m_tmpl : m_container->getTmplWidget(m_tmplFile);
}

uchar AlbumChildWidget::getLocations(uchar locations[]) const
{
    int size = m_photosList.size(), num = m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE] - size;

    memset(locations, 0, 2);

    //qDebug() << __FILE__ << __LINE__ << m_index << num << m_locations[PORTRAIT_PICTURE] << m_locations[LANDSCAPE_PICTURE] << m_photosList;

    if (0 < num)
    {
        locations[PORTRAIT_PICTURE] = m_locations[PORTRAIT_PICTURE];
        locations[LANDSCAPE_PICTURE] = m_locations[LANDSCAPE_PICTURE];

        if (size)
        {
            for (int i = 0; i < PHOTOS_NUMBER; i++)
            {
                int o = m_photoLabels.at(i)->getOrientation();
                if (PORTRAIT_PICTURE <= o && LANDSCAPE_PICTURE >= o)
                {
                    if (locations[o])
                    {
                        locations[o]--;
                    }
                    else
                    {
                        if (locations[o ^ 1])
                        {
                            locations[o ^ 1]--;
                        }
                    }
                }

                //qDebug() << __FILE__ << __LINE__ << m_index << o << locations[PORTRAIT_PICTURE] << locations[LANDSCAPE_PICTURE];
            }
        }

        //qDebug() << __FILE__ << __LINE__ << m_index << num;

        return num;
    }

    return locations[PORTRAIT_PICTURE] + locations[LANDSCAPE_PICTURE];
}

QSize AlbumChildWidget::getSize() const
{
    QVariantMap belongings = m_tmplLabel->getBelongings();
    return TemplateChildWidget::getSize(belongings["page_data"].toMap());
}

const QVariantList &AlbumChildWidget::getLayers() const
{
    QVariantMap &belongings = m_tmplLabel->getBelongings();
    QVariantMap page = belongings["page_data"].toMap();
    QVariantList info, layers = page["layers"].toList(), changes = m_records["photo_layers"].toList();

    //qDebug() << __FILE__ << __LINE__ << "before:" << changes;

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
    if (m_container)
    {
        m_container->onEdit(widgetsMap, m_index);
    }
}

int AlbumChildWidget::output(const QString &dir)
{
    //qDebug() << __FILE__ << __LINE__ << "output album" << m_index << TemplateChildWidget::isCover(getData()) << this->size() << getSize();

    int pid = 0;
    AlbumPageWidget *album = new AlbumPageWidget(PhotoLayer::VisiableImgTypeOriginal);

    if (album->loadLayers(*this) && (m_tmpl = m_container->getTmplWidget(m_tmplFile)))
    {
        QString path = tr("%1\\%2").arg(dir).arg(1 == m_index ? "cover" : "page");

        if (1 < m_index)
        {
            path += QString("%1").arg(m_index - 1);
        }

        QDir().mkpath(path);

        QFile jf(tr("%1\\%2").arg(path).arg(PAGE_DATA));
        if (jf.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QByteArray result = QtJson::serialize(getData());
            jf.write(result);
            jf.close();
        }

        //int pid = 0;
        uchar locations = m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE];
        if (!locations)
        {
            goto end;
        }

        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QStringList photoInfo = m_photosVector[i].split(TEXT_SEP);
            if (PHOTO_ATTRIBUTES != photoInfo.size())
            {
                continue;
            }

            QString photoFile = photoInfo.at(0);
            qreal angle = photoInfo.at(1).toDouble();
            Qt::Axis axis = (Qt::Axis)photoInfo.at(2).toInt();
            uint usedTimes = photoInfo.at(3).toUInt();

            //qDebug() << __FILE__ << __LINE__ << m_index << usedTimes << photoFile;

            do
            {
                if (album->exportPhoto(pid, photoFile, path, angle, axis))
                {
                    pid++;
                }
                else
                {
                    break;
                }

                if (!usedTimes)
                {
                    break;
                }
            } while (usedTimes > pid);
        }

        album->compose(pid, tr("%1\\preview.png").arg(path));

        QVariantMap pictures = m_tmpl->loadPictures();
        QVariantMap::const_iterator iter = pictures.constBegin();

        while (iter != pictures.constEnd())
        {
            QString file = tr("%1\\%2").arg(path).arg(iter.key());
            QPixmap pix;
            if (!QFile::exists(file) && pix.loadFromData(qUncompress(iter.value().toByteArray())))
            {
                pix.save(file);
            }
            ++iter;
        }
        //qDebug() << __FILE__ << __LINE__ << "changes:" << m_records["photo_layers"].toList();
    }

end:
    delete album;

    return pid;
}

inline void AlbumChildWidget::showPhotos(bool visible)
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
        showPhotos(true);
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
        showPhotos(false);
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
            showPhotos(!m_tmplVisible); // restore the visiable state of the tempalte
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
                event->ignore();
                return;
            }

            picLabel->accept();

            qreal angle = belongings["rotation_angle"].toReal();
            Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
            //int usedTimes = belongings["used_times"].toInt();

            m_picLabel->loadPicture(pix, m_thumbSize - QSize(2, 2), angle, axis);
            m_picLabel->setBelongings(belongings);
            m_picLabel->clearMimeType();

            m_photosList.append(picFile);
            if (0 == m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE])
            {
                addBanner(":/images/usages_num.png", "已用图片位");
            }
            else
            {
                changeBanners();
            }

            int index = m_picLabel->objectName().right(1).toInt() - 1;
            //m_photosVector[index] = QString("%1|%2|%3|%4|0").arg(picFile).arg(angle).arg(axis).arg(usedTimes);
            m_photosVector[index] = QString("%1|%2|%3|0").arg(picFile).arg(angle).arg(axis);
            //qDebug() << __FILE__ << __LINE__ << m_photosVector[index];

            m_container->noticeChanged();
        }
        else
        {
            if (1 == m_index && !TemplateChildWidget::isCover(belongings["page_data"].toMap()))
            {
                QMessageBox::information(parentWidget(),
                                         tr("操作失败"),
                                         tr("请拖入模板类型为手机封面（720x1080）的相册模板！"),
                                         tr("确定"));
                event->ignore();
                return;
            }

            if (picFile == m_tmplLabel->getPictureFile())
            {
                event->ignore();
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

            changeTemplate(belongings);
            //qDebug() << __FILE__ << __LINE__ << tmplFile;
        }

        showPhotos(!m_tmplVisible);

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

void AlbumChildWidget::replace(const QString &current, const QVariantMap &belongings)
{
    QString replaced = belongings["picture_file"].toString();
    if (m_photosList.contains(replaced, Qt::CaseInsensitive))
    {
        return;
    }

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        QStringList photoInfo = m_photosVector[i].split(TEXT_SEP);
        if (PHOTO_ATTRIBUTES == photoInfo.size() && current == photoInfo.at(0))
        {
            qreal angle = belongings["rotation_angle"].toReal();
            Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
            int usedTimes = photoInfo.at(3).toInt();

            m_photosList.removeOne(current);
            m_photosList.append(replaced);
            m_photosVector[i] = QString("%1|%2|%3|%4").arg(replaced).arg(angle).arg(axis).arg(usedTimes);
            m_photoLabels[i]->loadPicture(QPixmap(replaced), QSize(46, 46), angle, axis);
            m_photoLabels[i]->setBelongings(belongings);
        }
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
            showPhotos(tmplVisible);
        }
    }
    else
    {
        /* restore the view layer before entering */
        if (!m_locked)
        {
            showPhotos(tmplVisible);
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

    Converter::v2l(m_photosVector, photosList);
    m_records.insert("photos_list", photosList);
    m_records.insert("template_file", m_tmplFile);

    return PictureChildWidget::getChanges();
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
        m_pChildWidget->changeTemplate();
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

        m_pChildWidget->setPhotosVector(photosVector);
    }

    int n = Converter::num(m_pChildWidget->getPhotosList(), true);
    qDebug() << __FILE__ << __LINE__ << m_pChildWidget->getIndex() << n << m_pChildWidget->getPhotosList() << picFile;
    if (!n && !m_pChildWidget->getTmplLabel().hasPicture())
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
