#include "AlbumPageWidget.h"
#include "child/AlbumChildWidget.h"
//#include "child/TemplateChildWidget.h"
#include "child/ThumbChildWidget.h"
#include "defines.h"
#include <QDebug>

AlbumPageWidget::AlbumPageWidget(PhotoLayer::VisiableImgType type, QWidget *parent) :
    QWidget(parent), m_tmplWidget(NULL)
{
    m_bgdLabel = new BgdLayer(type, this);

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        m_layerLabels.insert(i, new PhotoLayer(type, this));
    }
}

bool AlbumPageWidget::loadLayers(const AlbumChildWidget &album)
{
    m_tmplWidget = album.getTmplWidget();
    if (!m_tmplWidget)
    {
        qDebug() << __FILE__ << __LINE__ << "doesn't find template" << album.getTmplFile();
        //m_pictures.clear();
        return false;
    }

#if !LOAD_FROM_MEMORY
    m_pictures = m_tmplWidget->loadPictures();
    if (m_pictures.isEmpty())
    {
        return false;
    }
#endif

    QString maskId, maskFile;
    QPoint bgdPos(0, 0);
    QSize bgdSize = album.getSize();
    QVariantList photoLayers, maskLayers, layers = album.getLayers();
    QVariantMap photoLayer, maskLayer, pictures = m_tmplWidget->loadPictures();
    PhotoLayer::VisiableImgType type = m_bgdLabel->getVisiableImgType();

    if (PhotoLayer::VisiableImgTypeOriginal == type)
    {
        setFixedSize(bgdSize);
        bgdPos.rx() = this->pos().x();
    }

    foreach (const QVariant &layer, layers)
    {
        QVariantMap embellishLayer, data = layer.toMap();
        QVariantMap frame = data["frame"].toMap();
        int width = frame["width"].toInt();
        int height = frame["height"].toInt();
        int x = frame["x"].toInt();
        int y = frame["y"].toInt();
        QString fileName = data["id"].toString() + ".png";
        qreal opacity = data["opacity"].toReal();
        qreal angle = data["rotation"].toReal();
        int type = data["type"].toInt();

        if (LT_Background == type)
        {
            QByteArray ba = qUncompress(pictures[fileName].toByteArray());

            if (bgdSize.width() == width && bgdSize.height() == height && x == width / 2 && y == height / 2)
            {
#if LOAD_FROM_MEMORY
                QPixmap bgdPix;
                if (!bgdPix.loadFromData(ba))
                {
                    continue;
                }
#else
                QPixmap *bgdPix = m_pictures["fileName"];
                if (bgdPix->isNull())
                {
                    continue;
                }
#endif

                m_bgdLabel->setActualSize(bgdSize);
                m_bgdLabel->loadPixmap(bgdPix);
                //m_bgdLabel->loadPixmap(QPixmap::fromImage(img));
                bgdSize = m_bgdLabel->getSize();
                if (PhotoLayer::VisiableImgTypeScreen == type)
                {
                    bgdPos.rx() = (this->width() - bgdSize.width()) / 2;
                }
                m_bgdLabel->setGeometry(QRect(bgdPos, bgdSize));
                m_bgdLabel->setAngle(angle);
                m_bgdLabel->setOpacity(opacity);
                //qDebug() << __FILE__ << __LINE__ << bgdSize << m_bgdLabel->geometry() << m_bgdLabel->getRatioSize() << bgdPix.isNull();
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
        else if (LT_Photo == type)
        {
            photoLayer.insert("frame", frame);
            photoLayer.insert("filename", fileName.replace(".png", PIC_FMT));
            //photoLayer.insert("picture", pictures[fileName].toByteArray());
            maskId = data["maskLayer"].toString();
            photoLayer.insert("maskfile", maskId.isEmpty() ? maskId : maskId + ".png");
            photoLayer.insert("opacity", opacity);
            photoLayer.insert("rotation", angle);
            photoLayer.insert("type", type);
            photoLayer.insert("used", 0);
            photoLayers << photoLayer;
            m_layers << photoLayer;
        }
        else if (LT_Mask == type)  // Mask layer
        {
            maskLayer.insert("frame", frame);
            maskLayer.insert("filename", fileName);
            maskLayer.insert("picture", qUncompress(pictures[fileName].toByteArray()));
            maskLayer.insert("opacity", opacity);
            maskLayer.insert("rotation", angle);
            maskLayers << maskLayer;
        }
    }

    foreach (const QVariant &layer, photoLayers)
    {
        photoLayer = layer.toMap();
        maskFile = photoLayer["maskfile"].toString();
        if (!maskFile.isEmpty())
        {
            for (int j = 0; j < maskLayers.size(); j++)
            {
                maskLayer = maskLayers.at(j).toMap();
                if (maskFile == maskLayer["filename"].toString())
                {
                    photoLayer.insert("maskLayer", maskLayer);
                    break;
                }
            }
        }

        m_photoLayers << photoLayer;
    }

    //qDebug() << __FILE__ << __LINE__ << m_photoLayers;

    return (!m_layers.isEmpty() && !m_photoLayers.isEmpty());
}

void AlbumPageWidget::clearLayers()
{
    m_layers.clear();
    m_photoLayers.clear();
    m_bgdLabel->flush();

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        m_layerLabels[i]->flush();
    }
}

QRect AlbumPageWidget::getLocation(const QString &layerId, const QString &maskFile) const
{
    QRect location;
    QVariantMap frame;

    if (maskFile.isEmpty() && !m_tmplWidget->getFrame(layerId, frame).isEmpty())
    {
        QSizeF ratioSize = m_bgdLabel->getRatioSize();
        int width = frame["width"].toInt() * ratioSize.width();
        int height = frame["height"].toInt() * ratioSize.height();
        int cx = frame["x"].toInt() * ratioSize.width();
        int cy = frame["y"].toInt() * ratioSize.height();
        location.setRect(cx - width / 2, cy - height / 2, width, height);
    }

    return location;
}

QRect AlbumPageWidget::getLocation(const QVariantMap &frame) const
{
    QRect location;

    if (!frame.isEmpty())
    {
        QSizeF ratioSize = m_bgdLabel->getRatioSize();
        int width = frame["width"].toInt() * ratioSize.width();
        int height = frame["height"].toInt() * ratioSize.height();
        int cx = frame["x"].toInt() * ratioSize.width();
        int cy = frame["y"].toInt() * ratioSize.height();
        location.setRect(cx - width / 2, cy - height / 2, width, height);
    }

    return location;
}

#if 0
bool AlbumPageWidget::loadPhoto(int index,
                                QVariantMap &record,
                                const QString &photoFile,
                                qreal angle,
                                Qt::Axis axis,
                                int id
                                )
{
    if (index < PHOTOS_NUMBER)
    //if (!pid && pid < m_photoLayers.size()) // for test
    {
        //qDebug() << __FILE__ << __LINE__ << index << photoFile << angle << axis;

        QString layerId, fileName;
        QVariantMap photoLayer, frame;
        int lid = 0, size = m_photoLayers.size();
        QRect location;
        bool restore = false, empty = record.isEmpty();

#if 1
        if (empty)
        {
            QPixmap pix(photoFile);
            if (angle || Qt::ZAxis != axis)
            {
                pix = pix.transformed(QTransform().rotate(angle, axis));
            }

            bool portrait = pix.width() < pix.height() ? true : false;
            for (int i = 0; i < size; i++)
            {
                photoLayer = m_photoLayers.at(i).toMap();
                bool used = photoLayer["used"].toBool();
                if (used)
                {
                    photoLayer.clear();
                    continue;
                }

                frame = photoLayer["frame"].toMap();
                int width = frame["width"].toInt();
                int height = frame["height"].toInt();
                if ((portrait && width < height) || (!portrait && width >= height))
                {
                    /*index = */lid = i;
                    break;
                }

                photoLayer.clear();
            }

            if (photoLayer.isEmpty())
            {
                for (int i = 0; i < size; i++)
                {
                    photoLayer = m_photoLayers.at(i).toMap();
                    if (!photoLayer["used"].toBool())
                    {
                        /*index = */lid = i;
                        break;
                    }

                    photoLayer.clear();
                }
            }

            if (!photoLayer.isEmpty())
            {
                fileName = photoLayer["filename"].toString();
                layerId = fileName.left(fileName.length() - strlen(PIC_FMT));
                record.insert("layer_id", layerId);
                location = getLocation(layerId, photoLayer["maskfile"].toString());
            }
        }
        else
#endif
        {
            //location = record["fixed_area"].toRect();
            layerId = record["layer_id"].toString();
            fileName = QString("%1%2").arg(layerId).arg(PIC_FMT);

            for (int i = 0; i < size; i++)
            {
                photoLayer = m_photoLayers.at(i).toMap();
                if (fileName == photoLayer["filename"].toString())
                {
                    if (!m_tmplWidget->getFrame(layerId, frame).isEmpty())
                    {
                        restore = photoLayer["frame"].toMap() != frame;
                        location = getLocation(frame);
                    }

                    lid = i;
                    break;
                }

                photoLayer.clear();
            }
        }

        //qDebug() << __FILE__ << __LINE__ << location << location.isNull();

        if (photoLayer.isEmpty() || (photoLayer["maskfile"].toString().isEmpty() && location.isNull()))
        {
            return false;
        }

        //qDebug() << __FILE__ << __LINE__ << usedTimes << photoLayer;
        qDebug() << __FILE__ << __LINE__ << lid << photoFile << layerId;

        photoLayer["used"] = 1;
        m_photoLayers[lid] = photoLayer;

        m_layerLabels[index]->setCanvas(m_bgdLabel->getRatioSize(), m_bgdLabel->geometry());
        return m_layerLabels[index]->loadPhoto(//m_photoLayers.first().toMap()
                                                   //m_photoLayers.last().toMap()
                                                   //m_photoLayers.at(index).toMap()
                                                   restore,
                                                   photoLayer,
                                                   photoFile,
                                                   angle,
                                                   axis,
                                                   location,
                                                   id
                                                   );
        //qDebug() << __FILE__ << __LINE__ << m_layerLabels[index]->geometry();
    }

    return false;
}

int AlbumPageWidget::loadPhotos(const QVariantList &photosInfo)
{
    int index = 0;

    if (photosInfo.isEmpty())
    {
        return 0;
    }

    int size = m_photoLayers.size();

    foreach (const QVariant &info, photosInfo)
    {
        QVariantMap data = info.toMap();
        if (data.isEmpty())
        {
            continue;
        }

        QString photoFile = data["picture_file"].toString();
        qreal angle = data["rotation_angle"].toReal();
        Qt::Axis axis = (Qt::Axis)data["rotation_axis"].toInt();
        QVariantList records = data["used_records"].toList();
        int usedTimes = records.size();
        QString layerId, fileName;

        if (usedTimes)
        {
            for (int j = 0; j < usedTimes; j++)
            {
                QVariantMap record = records[j].toMap();
                if (!record.isEmpty())
                {
                    //location = record["fixed_area"].toRect();
                    layerId = record["layer_id"].toString();
                    fileName = QString("%1%2").arg(layerId).arg(PIC_FMT);
                    for (int i = 0; i < size; i++)
                    {
                        QVariantMap photoLayer = m_photoLayers.at(i).toMap();
                        if (fileName == photoLayer["filename"].toString())
                        {
                            m_layerLabels[index]->loadPhoto(true,
                                                            m_photoLayers.at(index).toMap(),
                                                            photoFile,
                                                            angle,
                                                            axis,
                                                            getLocation(layerId, photoLayer["maskfile"].toString())
                                    );
                            index++;
                            break;
                        }
                    }
                }
            }
        }

        qDebug() << __FILE__ << __LINE__ << photoFile << "is used" << usedTimes << "on layer" << layerId;
    }

    return index;
}
#endif

int AlbumPageWidget::loadPhotos(QVariantList &photosInfo,
                                int totalTimes,
                                int photosNum,
                                const QString &savePath)
{
    int pid = 0, size = m_photoLayers.size();
    QSizeF ratio = m_bgdLabel->getRatioSize();
    QRect location, rect = m_bgdLabel->geometry();
    bool restore = false;
    QVariantMap photoLayer, frame, record;
    QString fileName, layerId;

    if (!totalTimes)
    {
        for (int i = 0; i < size; i++)
        {
            photoLayer = m_photoLayers.at(i).toMap();
            if (photoLayer["used"].toBool())
            {
                continue;
            }

            frame = photoLayer["frame"].toMap();
            bool portrait = frame["width"].toInt() < frame["height"].toInt() ? true : false;
            bool ssel = false;  // sel-select
            QVariantMap info;
            QVariantList records;
            QString photoFile;
            qreal angle = 0;
            Qt::Axis axis = Qt::ZAxis;
            int j = 0;

            do
            {
                if (j == PHOTOS_NUMBER - 1)
                {
                    ssel = true;
                    j = 0;
                }

                info = photosInfo[j].toMap();
                if (info.isEmpty())
                {
                    info.clear();
                    continue;
                }

                records = info["used_records"].toList();
                if (!records.isEmpty())
                {
                    info.clear();
                    continue;
                }

                photoFile = info["picture_file"].toString();
                angle = info["rotation_angle"].toReal();
                axis = (Qt::Axis)info["rotation_axis"].toInt();

                if (ssel)
                {
                    break;
                }

                QPixmap pix(photoFile);
                if (angle || Qt::ZAxis != axis)
                {
                    pix = pix.transformed(QTransform().rotate(angle, axis));
                }

                if ((portrait && pix.width() < pix.height()) || (!portrait && pix.width() >= pix.height()))
                {
                    break;
                }
            } while (PHOTOS_NUMBER > j++);

            if (!info.isEmpty())
            {
                fileName = photoLayer["filename"].toString();
                layerId = fileName.left(fileName.length() - strlen(PIC_FMT));
                location = getLocation(layerId, photoLayer["maskfile"].toString());

                record.insert("layer_id", layerId);
                info.insert("used_records", records << record);
                photosInfo[j] = info;

                photoLayer["used"] = 1;
                m_photoLayers[i] = photoLayer;

                m_layerLabels[i]->setCanvas(ratio, rect);
                if (m_layerLabels[i]->loadPhoto(restore, photoLayer, photoFile, angle, axis, location, i))
                {
                    if (!savePath.isEmpty())
                    {
                        exportPhoto(m_layerLabels[i]->getPicture(false).toImage(), savePath, m_layerLabels[i]->getPictureFile());
                    }

                    pid++;
                }
            }
        }
    }
    else
    {
        //qDebug() << __FILE__ << __LINE__ << photosInfo;

        for (int i = 0; i < size; i++)
        {
            photoLayer = m_photoLayers.at(i).toMap();
            //qDebug() << __FILE__ << __LINE__ << photoLayer;

            photoLayer["used"] = 0;

            fileName = photoLayer["filename"].toString();
            layerId = fileName.left(fileName.length() - strlen(PIC_FMT));

            if (!m_tmplWidget->getFrame(layerId, frame).isEmpty())
            {
                restore = photoLayer["frame"].toMap() != frame;
                if (photoLayer["maskfile"].toString().isEmpty())
                {
                    location = getLocation(frame);
                }
            }

            QString photoFile;
            qreal angle = 0;
            Qt::Axis axis = Qt::ZAxis;

            for (int j = 0; j < PHOTOS_NUMBER; j++)
            {
                QVariantMap info = photosInfo[j].toMap();
                if (info.isEmpty())
                {
                    continue;
                }

                QVariantList records = info["used_records"].toList();
                if (records.isEmpty())
                {
                    continue;
                }

                int usedTimes = records.size();
                for (int k = 0; k < usedTimes; k++)
                {
                    QVariantMap record = records[k].toMap();
                    if (layerId == record["layer_id"].toString())
                    {
                        photoLayer["used"] = 1;
                        photoFile = info["picture_file"].toString();
                        angle = info["rotation_angle"].toReal();
                        axis = (Qt::Axis)info["rotation_axis"].toInt();
                        break;
                    }
                }

                if (photoLayer["used"].toBool())
                {
                    break;
                }
            }

            m_layerLabels[i]->setCanvas(ratio, rect);

            if (m_layerLabels[i]->loadPhoto(restore, photoLayer, photoFile, angle, axis, location, i))
            {
                m_photoLayers[i] = photoLayer;

                if (!savePath.isEmpty())
                {
                    exportPhoto(m_layerLabels[i]->getPicture(false).toImage(), savePath, m_layerLabels[i]->getPictureFile());
                }

                pid++;
            }
        }

        if (totalTimes < photosNum && totalTimes < m_tmplWidget->getLocations())
        {
            loadPhotos(photosInfo, 0, photosNum, savePath);
        }
    }

    return pid;
}

inline void AlbumPageWidget::exportPhoto(const QImage &image, const QString &savePath, const QString &fileName)
{
    if (image.isNull() || savePath.isEmpty() || fileName.isEmpty())
    {
        return;
    }

    QSize size = image.size();

    if (MAX_PIC_SIZE < size.width())
    {
        size.setWidth(MAX_PIC_SIZE);
    }

    if (MAX_PIC_SIZE < size.width())
    {
        size.setHeight(MAX_PIC_SIZE);
    }

    //qDebug() << __FILE__ << __LINE__ << index << m_layerLabels[index]->getPictureFile() << angle;
    image.scaled(size, Qt::KeepAspectRatio).save(tr("%1\\%2").arg(savePath).arg(fileName));
}

void AlbumPageWidget::removePhoto(const QString &picFile)
{
    //bool ok = false;

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        if (picFile == m_layerLabels[i]->getPhotoFile())
        {
            //ok = true;

            QString fileName = m_layerLabels[i]->getPictureFile();
            int size = m_photoLayers.size();

            m_layerLabels[i]->flush(false);

            for (int i = 0; i < size; i++)
            {
                QVariantMap photoLayer = m_photoLayers.at(i).toMap();
                if (fileName == photoLayer["filename"].toString() && photoLayer["used"].toBool())
                {
                    photoLayer["used"] = 0;
                    m_photoLayers[i] = photoLayer;
                    break;
                }
            }
        }
    }

    //qDebug() << __FILE__ << __LINE__ << picFile << m_photoLayers;

//    if (ok)
//    {
//        compose();
//    }
}

void AlbumPageWidget::sort(QVariantList &records)
{
    int count = records.size();
    if (1 >= count)
    {
        return;
    }

    qDebug() << __FILE__ << __LINE__ << "before:" << records;

    for (int i = 0; i < count; i++)
    {
        for (int j = count - 1; j > i; j--)
        {
            QVariantMap frame1, record1 = records[j - 1].toMap();
            QString layerId1 = record1["layer_id"].toString();
            if (m_tmplWidget->getFrame(layerId1, frame1).isEmpty())
            {
                return;
            }

            QRect location1 = getLocation(frame1);
            QVariantMap frame2, record2 = records[j].toMap();
            QString layerId2 = record2["layer_id"].toString();
            if (m_tmplWidget->getFrame(layerId2, frame2).isEmpty())
            {
                return;
            }

            QRect location2 = getLocation(frame2);
            if (location1.width() < location2.width() && location1.height() < location2.height())
            {
                records[j - 1] = record2;
                records[j] = record1;
            }
        }
    }

    qDebug() << __FILE__ << __LINE__ << "after:" << records;
}

void AlbumPageWidget::replace(AlbumChildWidget &album, const ThumbChildWidget *thumb, PhotoLayer *label)
{
    if (thumb)
    {
        LabelsVector labels;

        if (!label)
        {
            QString picFile = thumb->getPictureLabel()->getPictureFile();
            for (int i = 0; i < PHOTOS_NUMBER; i++)
            {
                if (picFile == m_layerLabels[i]->getPhotoFile())
                {
                    labels << m_layerLabels[i];
                }
            }
        }
        else
        {
            label->setThumbId(thumb->getIndex());
            labels << label;
        }

        //qDebug() << __FILE__ << __LINE__ << labels.size();

        for (int i = 0; i < labels.size(); i++)
        {
            if (!label)
            {
                QVariantMap belongings = thumb->getBelongings();
                QString picFile = belongings["picture_file"].toString();
                qreal angle = belongings["rotation_angle"].toReal();
                Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
                labels[i]->changePhoto(picFile, angle, axis);
            }

            compose();

            QString layerId;
            Converter::getFileName(labels[i]->getPictureFile(), layerId, false);
            //qDebug() << __FILE__ << __LINE__ << i << picFile << labels[i]->getFrame();
            album.changePhoto(layerId, labels[i]->getFrame(), labels[i]->getOpacity(), labels[i]->getAngle());
        }
    }
}

void AlbumPageWidget::compose(int locations, const QString &saveFile)
{
    if (0 <= locations)
    {
        m_bgdLabel->loadLayers(locations, m_layers, m_layerLabels);
    }

    m_bgdLabel->compose(saveFile);
}
