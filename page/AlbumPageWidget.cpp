#include "AlbumPageWidget.h"
#include "child/AlbumChildWidget.h"
#include "child/TemplateChildWidget.h"
#include "child/ThumbChildWidget.h"
#include "defines.h"
#include <QDebug>

AlbumPageWidget::AlbumPageWidget(PhotoLayer::VisiableImgType type, QWidget *parent) :
    QWidget(parent)
{
    m_bgdLabel = new BgdLayer(type, this);

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        m_layerLabels.insert(i, new PhotoLayer(type, this));
    }
}

bool AlbumPageWidget::loadLayers(const AlbumChildWidget &album)
{
    TemplateChildWidget *tmplWidget = album.getTmplWidget();
    if (!tmplWidget)
    {
        qDebug() << __FILE__ << __LINE__ << "doesn't find template" << album.getTmplFile();
        return false;
    }

    QString maskFile;
    QPoint bgdPos(0, 0);
    QSize bgdSize = album.getSize();
    QVariantList photoLayers, maskLayers, layers = album.getLayers();
    QVariantMap photoLayer, maskLayer, pictures = tmplWidget->loadPictures();
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
                QPixmap bgdPix;
                if (!bgdPix.loadFromData(ba))
                {
                    continue;
                }

                m_bgdLabel->setActualSize(bgdSize);
                m_bgdLabel->loadPixmap(bgdPix);
                bgdSize = m_bgdLabel->getSize();
                if (PhotoLayer::VisiableImgTypeScreen == type)
                {
                    bgdPos.rx() = (this->width() - bgdSize.width()) / 2;
                }
                m_bgdLabel->setGeometry(QRect(bgdPos, bgdSize));
                m_bgdLabel->setAngle(angle);
                m_bgdLabel->setOpacity(opacity);
                //qDebug() << __FILE__ << __LINE__ << bgdSize << m_bgdLabel->geometry() << m_bgdLabel->getRatioSize();
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
            photoLayer.insert("maskfile", data["maskLayer"].toString() + ".png");
            photoLayer.insert("opacity", opacity);
            photoLayer.insert("rotation", angle);
            photoLayer.insert("type", type);
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

bool AlbumPageWidget::loadPhoto(int index,
                                const QString &photoFile,
                                qreal angle,
                                Qt::Axis axis,
                                int id)
{
    if (index < m_photoLayers.size())
    //if (!pid && pid < m_photoLayers.size()) // for test
    {
        //qDebug() << __FILE__ << __LINE__ << photoFile << angle << axis;
        m_layerLabels[index]->setCanvas(m_bgdLabel->getRatioSize(), m_bgdLabel->geometry());
        return m_layerLabels[index]->loadPhoto(//m_photoLayers.first().toMap()
                                            //m_photoLayers.last().toMap()
                                              m_photoLayers.at(index).toMap()
                                              ,photoFile,
                                              angle,
                                              axis,
                                              id
                                              );
    }

    return false;
}

int AlbumPageWidget::loadPhotos(const QStringList &photosList)
{
    int index = 0;

    if (!photosList.isEmpty())
    {
        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            QString photoAttr = photosList.at(i);
            if (photoAttr.isEmpty())
            {
                continue;
            }

            QStringList photoInfo = photoAttr.split(TEXT_SEP);
            if (PHOTO_ATTRIBUTES == photoInfo.size() && index < m_photoLayers.size())
            {
                m_layerLabels[index]->loadPhoto(m_photoLayers.at(index).toMap(),
                                                photoInfo.at(0),
                                                photoInfo.at(1).toDouble(),
                                                (Qt::Axis)photoInfo.at(2).toInt());

//                QString picFile = photoInfo.at(0);
//                qreal angle = photoInfo.at(1).toDouble();
//                Qt::Axis axis = (Qt::Axis)photoInfo.at(2).toInt();
//                qDebug() << __FILE__ << __LINE__ << picFile << angle << axis;
//                m_layerLabels[i]->changePhoto(photoInfo.at(0), photoInfo.at(1).toDouble(), (Qt::Axis)photoInfo.at(2).toInt());

                index++;
            }
        }
    }

    return index;
}

bool AlbumPageWidget::exportPhoto(int index,
                                  const QString &fileName,
                                  const QString &savePath,
                                  qreal angle,
                                  Qt::Axis axis)
{
    if (loadPhoto(index, fileName, angle, axis))
    {
        QPixmap pix = m_layerLabels[index]->getPicture(false);
        QSize size = pix.size();

        if (MAX_PIC_SIZE < size.width())
        {
            size.setWidth(MAX_PIC_SIZE);
        }

        if (MAX_PIC_SIZE < size.width())
        {
            size.setHeight(MAX_PIC_SIZE);
        }

        //qDebug() << __FILE__ << __LINE__ << index;
        return pix.scaled(size, Qt::KeepAspectRatio).save(tr("%1\\%2").arg(savePath).arg(m_layerLabels[index]->getPictureFile()));
    }

    return false;
}

void AlbumPageWidget::removePhoto(const QString &fileName)
{
    bool ok = false;

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        if (fileName == m_layerLabels[i]->getPhotoFile())
        {
            ok = true;
            m_layerLabels[i]->flush(false);
        }
    }

    if (ok)
    {
        compose();
    }
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
            label->setId(thumb->getIndex());
            labels << label;
        }

        //qDebug() << __FILE__ << __LINE__ << labels.size();

        for (int i = 0; i < labels.size(); i++)
        {
            QVariantMap belongings = thumb->getBelongings();
            QString picFile = belongings["picture_file"].toString();
            qreal angle = belongings["rotation_angle"].toReal();
            Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();

            labels[i]->changePhoto(picFile, angle, axis);
            compose();

            QString photoName;
            Converter::getFileName(labels[i]->getPictureFile(), photoName, false);
            //qDebug() << __FILE__ << __LINE__ << i << picFile << photoName << labels[i]->getFrame();
            album.changePhoto(photoName, labels[i]->getFrame(), labels[i]->getOpacity(), labels[i]->getAngle());
        }
    }
}

void AlbumPageWidget::compose(int count, const QString &fileName)
{
    if (0 <= count)
    {
        m_bgdLabel->loadLayers(count, m_layers, m_layerLabels);
    }

    m_bgdLabel->compose(fileName);
}

void AlbumPageWidget::showPage(bool visiable)
{
    m_bgdLabel->setVisible(visiable);

    for (int i = 0; i < PHOTOS_NUMBER; i++)
    {
        m_layerLabels[i]->setVisible(visiable);
    }

    setVisible(visiable);
}
