#include "AlbumChildWidget.h"
#include "ui_AlbumChildWidget.h"
#include "PhotoChildWidget.h"
//#include "TemplateChildWidget.h"
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
    m_tmplVisible(false),
    m_tmpl(NULL)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    setupWidgets();
}

AlbumChildWidget::AlbumChildWidget(int index,
                                   const QVariantList &photosInfo,
                                   const QString &tmplFile,
                                   const QVariantList &photoLayers,
                                   TaskPageWidget *parent) :
    PictureChildWidget(tr("第 %1 页（%2）").arg(index).arg(1 == index ? tr("封面") : tr("内页")), QSize(118, 190), true, parent),
    ui(new Ui::AlbumChildWidget),
    m_tmplVisible(false),
    m_tmpl(NULL)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    setupWidgets(photosInfo, tmplFile, photoLayers);

    if (m_tmplLabel->hasPicture())
    {
        ui->photoLabel1->setVisible(false);
        ui->photoLabel2->setVisible(false);
        ui->photoLabel3->setVisible(false);
        ui->photoLabel4->setVisible(false);
        ui->photoLabel5->setVisible(false);
        ui->photoLabel6->setVisible(false);
        m_tmplLabel->setVisible(true);
    }
}

AlbumChildWidget::~AlbumChildWidget()
{
    delete ui;
}

inline DraggableLabel *AlbumChildWidget::setPictureLabel(QLabel *label)
{
    Q_ASSERT(label);

    DraggableLabel *picLabel = new DraggableLabel(QSize(48, 48), DRAGGABLE_PHOTO, label);
    connect(picLabel, SIGNAL(clicked()), SIGNAL(itemSelected()));
    connect(picLabel, SIGNAL(dblClicked()), SIGNAL(itemDblSelected()));

    return picLabel;
}

void AlbumChildWidget::setupWidgets(const QVariantList &photosInfo,
                                    const QString &tmplFile,
                                    const QVariantList &photoLayers)
{
    int j = 0, size = photosInfo.size();
    bool empty = photosInfo.isEmpty();

    if (PHOTOS_NUMBER != size)
    {
        size = PHOTOS_NUMBER;
    }

    memset(m_locations, 0, 2);

    m_photoLabels.append(setPictureLabel(ui->photoLabel1));
    m_photoLabels.append(setPictureLabel(ui->photoLabel2));
    m_photoLabels.append(setPictureLabel(ui->photoLabel3));
    m_photoLabels.append(setPictureLabel(ui->photoLabel4));
    m_photoLabels.append(setPictureLabel(ui->photoLabel5));
    m_photoLabels.append(setPictureLabel(ui->photoLabel6));

    for (int i = 0; i < size; i++)
    {
        /* Initialize the persent album with the photos list */
        QVariantMap info;
        if (empty)
        {
            m_photosInfo.append(info);
        }
        else
        {
            if (PHOTOS_NUMBER > i)
            {
                info = photosInfo[i].toMap();
                if (!info.isEmpty())
                {
                    QString photoFile = info["picture_file"].toString();
                    if (QFile::exists(photoFile))
                    {
                        qreal angle = info["rotation_angle"].toReal();
                        Qt::Axis axis = (Qt::Axis)info["rotation_axis"].toInt();
                        m_photoLabels[j]->loadPicture(QPixmap(photoFile), QSize(46, 46), angle, axis);
                        PhotoChildWidget::setPhotoInfo(*m_photoLabels[i], photoFile, angle, axis);
                    }
                    else
                    {
                        size++;
                        continue;
                    }

                    //qDebug() << __FILE__ << __LINE__ << i << j << photoFile; // << info["used_records"].toList()
                }
            }

            m_photosInfo.insert(j, info);
        }

        m_photoLabels[j]->setContentsMargins(1, 1, 2, 2);
        j++;
    }

    //qDebug() << __FILE__ << __LINE__ << m_index << photoFile << records;

    m_picLabel = m_photoLabels.last();
    m_tmplLabel = new DraggableLabel(QSize(96, 144), DRAGGABLE_TEMPLATE, ui->templateLabel);

    if (!tmplFile.isEmpty())
    {
        TemplateChildWidget tmplWidget(tmplFile, m_tmplLabel, m_container);
        if (QFile::exists(tmplFile))
        {
            QString tmplPic;
            if (tmplWidget.getTmplPic(tmplPic))
            {
                m_tmplFile = tmplFile;
                m_tmplLabel->setContentsMargins(1, 1, 2, 2);
                m_tmplLabel->loadPicture(QPixmap(tmplPic), QSize(92, 142));
                QVariantMap belongings = m_tmplLabel->getBelongings();
                TemplateChildWidget::getLocations(belongings["page_data"].toMap(), m_locations);
            }
        }
        else
        {
            tmplWidget.remove();
        }
    }

    //qDebug() << __FILE__ << __LINE__ << m_index << m_tmplFile;

    m_tmplLabel->setVisible(false);
    connect(m_tmplLabel, SIGNAL(clicked()), SIGNAL(itemSelected()));
    connect(m_tmplLabel, SIGNAL(dblClicked()), SIGNAL(itemDblSelected()));

    ui->innerHorizontalLayout->setAlignment(Qt::AlignLeft);
    changeBanners();

    m_records.insert("photo_layers", photoLayers);
    m_records.insert("photos_info", m_photosInfo);
    m_records.insert("template_file", m_tmplFile);
}

bool AlbumChildWidget::setPhotoInfo(DraggableLabel *picLabel, QVariantMap info)
{
    Q_ASSERT(picLabel);

    QVariantMap belongings = picLabel->getBelongings();
    if (belongings.isEmpty())
    {
        return false;
    }

    bool empty = info.isEmpty();
    QString picFile = belongings["picture_file"].toString();
    QPixmap pix(picFile);

    if (empty)
    {
        int index = getPhotoIndex(picFile);
        if (INVALID_PHOTO_INDEX != index || pix.isNull())
        {
            return false;
        }
    }

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        if (!m_photoLabels[i]->hasPicture())
        {
            qreal angle = belongings["rotation_angle"].toReal();
            Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();

            if (empty)
            {
                info.insert("picture_file", picFile);
                info.insert("rotation_angle", angle);
                info.insert("rotation_axis", axis);
                info.insert("used_records", QVariantList());
                m_photosInfo[i] = info;

                if (0 == m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE])
                {
                    addBanner(":/images/usages_num.png", "已用图片位");
                }
                else
                {
                    changeBanners();
                }

                picLabel->accept();
                m_container->noticeChanged();
            }
            else
            {
                m_photosInfo[i] = info;
            }

            m_picLabel = m_photoLabels[i];
            m_picLabel->loadPicture(pix, QSize(46, 46), angle, axis);
            m_picLabel->setBelongings(belongings);
            m_picLabel->clearMimeType();

            return true;
        }
    }

    return false;
}

void AlbumChildWidget::resetPhotosInfo()
{
    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        QVariantMap info = m_photosInfo[i].toMap();
        if (info.isEmpty())
        {
            continue;
        }

        QVariantList records = info["used_records"].toList();
        if (records.isEmpty())
        {
            continue;
        }

        records.clear();
        info.insert("used_records", records);
        m_photosInfo[i] = info;
    }
}

const QStringList &AlbumChildWidget::getPhotosList()
{
    m_photosList.clear();

    foreach (const QVariant &info, m_photosInfo)
    {
        QVariantMap data = info.toMap();
        if (data.isEmpty())
        {
            continue;
        }

        QString fileName = data["picture_file"].toString();
        if (!fileName.isEmpty())
        {
            m_photosList << fileName;
        }
    }

    return m_photosList;
}

bool AlbumChildWidget::addUsedRecord(QVariantList &records, const QString &layerId)
{
    if (layerId.isEmpty())
    {
        return false;
    }

    QVariantMap record;
    int num = records.size();

    for (int i = 0; i < num; i++)
    {
        record = records.at(i).toMap();
        if (!record.isEmpty() && layerId == record["layer_id"].toString())
        {
            return false;
        }

        record.clear();
    }

    if (record.isEmpty())
    {
        record.insert("layer_id", layerId);
    }

    records << record;

    return true;
}

void AlbumChildWidget::removeUsedRecord(QVariantList &records, const QString &layerId)
{
    int num = records.size();

    for (int i = 0; i < num; i++)
    {
        QVariantMap record = records.at(i).toMap();
        if (!record.isEmpty() && layerId == record["layer_id"].toString())
        {
            records.removeAt(i);
            break;
        }
    }
}

int AlbumChildWidget::getPhotoIndex(const QString &fileName)
{
    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        QVariantMap info = m_photosInfo[i].toMap();
        if (info.isEmpty())
        {
            continue;
        }

        if (fileName == info["picture_file"].toString())
        {
            return i;
        }
    }

    return INVALID_PHOTO_INDEX;
}

int AlbumChildWidget::getPhotosNum() const
{
    int num = 0;

    foreach (const QVariant &info, m_photosInfo)
    {
        QVariantMap data = info.toMap();
        if (!data.isEmpty() && !data["picture_file"].toString().isEmpty())
        {
            num++;
        }
    }

    return num;
}

int AlbumChildWidget::getTotalUsedTimes() const
{
    int num = 0;

    foreach (const QVariant &info, m_photosInfo)
    {
        int times = 0;
        QVariantMap data = info.toMap();
        if (!data.isEmpty() && (times = data["used_records"].toList().size()))
        {
            num += times;
        }
    }

    return num;
}

void AlbumChildWidget::changeBanners()
{
    int usages = getPhotosNum();
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

    resetPhotosInfo();

    if (belongings.isEmpty())
    {
        m_tmplFile.clear();
        m_tmplLabel->flush();
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

void AlbumChildWidget::changePhoto(const QString &layerId,
                                   QRect rect,
                                   qreal opacity,
                                   qreal angle)
{
    QVariantMap &belongings = m_tmplLabel->getBelongings();
    QVariantMap page = belongings["page_data"].toMap();
    QVariantList info, layers = page["layers"].toList(), changes = m_records["photo_layers"].toList();
    bool changed = false;

    //qDebug() << __FILE__ << __LINE__ << "before:" << opacity;
    //qDebug() << __FILE__ << __LINE__ << "before:" << layerId << rect;

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        if (layerId == data["id"].toString())
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
                if (layerId == related["id"].toString())
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

    if (changed)
    {
        //qDebug() << __FILE__ << __LINE__ << "after:" << changes;
        m_records.insert("photo_layers", changes);
        m_container->noticeChanged();
    }
}

void AlbumChildWidget::changePhoto(const QString &layerId, const QString &fileName)
{
    QVariantMap &belongings = m_tmplLabel->getBelongings();
    QVariantMap page = belongings["page_data"].toMap();
    QVariantList info, layers = page["layers"].toList();

    //qDebug() << __FILE__ << __LINE__ << "before:" << opacity;
    //qDebug() << __FILE__ << __LINE__ << "before:" << layerId << rect;

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        if (layerId == data["id"].toString())
        {
            data.insert("ext", 1);
            data.insert("ref", fileName);
            data.insert("reftype", 0);
            //qDebug() << __FILE__ << __LINE__ << "found";
        }

        info << data;
    }

    page.insert("layers", info);
    belongings.insert("page_data", page);
}

bool AlbumChildWidget::removePhoto(const QString &fileName)
{
    bool ok = false;
    int index = getPhotoIndex(fileName);

    if (INVALID_PHOTO_INDEX != index)
    {
        ok = true;
        m_photoLabels[index]->flush();
        m_photosInfo[index].clear();

        for (int i = index + 1; i < PHOTOS_NUMBER; i++)
        {
            QVariantMap info = m_photosInfo[i].toMap();
            if (info.isEmpty())
            {
                continue;
            }

            setPhotoInfo(m_photoLabels[i], info);
            m_photoLabels[i]->flush();
            m_photosInfo[i].clear();
        }

        changeBanners();
    }

    return ok;
}

TemplateChildWidget *AlbumChildWidget::getTmplWidget() const
{
    return m_tmpl ? m_tmpl : m_container->getTmplWidget(m_tmplFile);
}

uchar AlbumChildWidget::getLocations(uchar locations[]) const
{
    int num = getPhotosNum();
    int count = m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE] - num;

    memset(locations, 0, 2);

    //qDebug() << __FILE__ << __LINE__ << m_index << num << m_locations[PORTRAIT_PICTURE] << m_locations[LANDSCAPE_PICTURE] << m_photosList;

    if (0 < count)
    {
        locations[PORTRAIT_PICTURE] = m_locations[PORTRAIT_PICTURE];
        locations[LANDSCAPE_PICTURE] = m_locations[LANDSCAPE_PICTURE];

        if (num)
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
        QString layerId = data["id"].toString();

        foreach (const QVariant &change, changes)
        {
            QVariantMap related = change.toMap();
            if (layerId == related["id"].toString())
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

    int pid = INVALID_PHOTO_INDEX;
    AlbumPageWidget *album = new AlbumPageWidget(PhotoLayer::VisiableImgTypeOriginal);

    if (album->loadLayers(*this) && (m_tmpl = m_container->getTmplWidget(m_tmplFile)))
    {
        uchar locations = m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE];
        if (!locations)
        {
            goto end;
        }

        QString path = tr("%1\\%2").arg(dir).arg(1 == m_index ? "cover" : "page");
        if (1 < m_index)
        {
            path += QString("%1").arg(m_index - 1);
        }

        QDir().mkpath(path);

        pid = 0;

        if (!getPhotosNum())
        {
            QString fileName = tr("%1\\%2").arg(path).arg(PIC_NAME);
            QString tmplPic;

            if (m_tmpl->getTmplPic(tmplPic))
            {
                QFile::copy(tmplPic, fileName);
            }
            else
            {
                QFile file(fileName);
                file.open(QIODevice::WriteOnly);
                file.close();
            }
        }
        else
        {
            pid = album->loadPhotos(*this, m_photosInfo, getTotalUsedTimes(), path);
            album->compose(pid, tr("%1\\%2").arg(path).arg(PIC_NAME));
        }

        //qDebug() << __FILE__ << __LINE__ << "photosInfo:" << m_photosInfo;

        QVariantMap pictures = m_tmpl->loadPictures();
        QVariantMap::const_iterator iter = pictures.constBegin();
        //PicturesMap pictures = m_tmpl->loadPictures();
        //PicturesMap::const_iterator iter = pictures.constBegin();

        while (iter != pictures.constEnd())
        {
            QString file = tr("%1\\%2").arg(path).arg(iter.key());

#if LOAD_FROM_MEMORY
            QPixmap pix;
            if (!QFile::exists(file) && pix.loadFromData(qUncompress(iter.value().toByteArray())))
            {
                pix.save(file);
            }
#else
            //QImage img = iter.value();
            QPixmap *img = iter.value();
            img->save(file);
#endif

            ++iter;
        }

        //qDebug() << __FILE__ << __LINE__ << "changes:" << m_records["photo_layers"].toList();

        QFile jf(tr("%1\\%2").arg(path).arg(PAGE_DATA));
        if (jf.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QByteArray result = QtJson::serialize(getData());
            jf.write(result);
            jf.close();
        }
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
                m_picLabel = photoLabel;
                return true;
            }
        }
    }
    else if (event->mimeData()->hasFormat(DRAGGABLE_TEMPLATE))
    {
        showPhotos(false);
        m_picLabel = m_tmplLabel;
        return true;
    }

    m_picLabel = NULL;
    event->ignore();

    return false;
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
            if (!setPhotoInfo(picLabel))
            {
                event->ignore();
                return;
            }

            m_tmplVisible = m_tmplLabel->hasPicture();
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

            if (1 < m_index && TemplateChildWidget::isCover(belongings["page_data"].toMap()))
            {
                QMessageBox::information(parentWidget(),
                                         tr("操作失败"),
                                         tr("请拖入模板类型为手机内页的相册模板！"),
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
            //showPhotos(false);
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
    }
}

void AlbumChildWidget::replace(const QString &current, const QVariantMap &belongings)
{
    QString replaced = belongings["picture_file"].toString();
    int index = getPhotoIndex(replaced);
    if (INVALID_PHOTO_INDEX != index)   // existing?
    {
        return;
    }

    index = getPhotoIndex(current);
    if (INVALID_PHOTO_INDEX != index)   // not existing?
    {
        QVariantMap info = m_photosInfo[index].toMap();
        qreal angle = info["rotation_angle"].toReal();
        Qt::Axis axis = (Qt::Axis)info["rotation_axis"].toInt();

        info["picture_file"] = replaced;
        m_photosInfo[index] = info;
        m_photoLabels[index]->loadPicture(QPixmap(replaced), QSize(46, 46), angle, axis);
        m_photoLabels[index]->setBelongings(belongings);
    }
}

void AlbumChildWidget::switchView(bool enter)
{
    bool sw = m_tmplLabel->hasPicture();

    if (enter)
    {
        if (sw)
        {
            showPhotos(true);
            return;
        }
    }
    else
    {
        if (sw)
        {
            showPhotos(false);
            return;
        }
    }

    showPhotos(!ui->photoLabel1->isVisible() && ui->templateLabel->isVisible());
}

const QVariantMap &AlbumChildWidget::getChanges()
{
    m_records.insert("photos_info", m_photosInfo);
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
    bool ok = true;

    /* Remove one picture(maybe it is a photo or template type) file from the specified album */
    if (picFile == m_pChildWidget->getTmplLabel().getPictureFile())
    {
        m_pChildWidget->changeTemplate();
    }
    else
    {
        ok = m_pChildWidget->removePhoto(picFile);
    }

    if (!m_pChildWidget->getPhotosNum() && !m_pChildWidget->getTmplLabel().hasPicture())
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
