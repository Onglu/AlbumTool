#include "BgdLayer.h"
#include <QPainter>
#include <QDebug>

void BgdLayer::loadPixmap(const QPixmap &pix)
{
    QSize size = parentWidget() ? parentWidget()->size() : this->size();

    //qDebug() << __FILE__ << __LINE__ << size;

    if (loadPicture(pix, size))
    {
        if (PhotoLayer::VisiableImgTypeScreen == m_type)
        {
            m_srcImg = m_bk.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            m_ratioSize.setWidth((qreal)m_size.width() / m_actualSize.width());
            m_ratioSize.setHeight((qreal)m_size.height() / m_actualSize.height());
        }
        else
        {
            m_srcImg = m_ori.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            m_ratioSize = QSizeF(1, 1);
        }
    }
}

void BgdLayer::compose(const QString &fileName)
{
    if (VISIABLE_IMG_TYPES <= m_type || m_srcImg.isNull())
    {
        return;
    }

    QImage composedImg = m_srcImg;

    int index = 0;
    QPainter painter(&composedImg);
    painter.fillRect(composedImg.rect(), Qt::transparent);

    foreach (const QVariant &layer, m_layers)
    {
        QVariantMap data = layer.toMap();
        QString file = data["filename"].toString();
        qreal opacity = data["opacity"].toReal();
        qreal angle = data["angle"].toReal();

        //qDebug() << __FILE__ << __LINE__ << "layer:" << filename;

        if (1 == data["type"].toInt())
        {
            if (m_locations == index)
            {
                continue;
            }

            PhotoLayer *label = m_labels.at(index);
            //qDebug() << __FILE__ << __LINE__ << index << label->hasPicture();
            if (label && label->hasPhoto())
            {
                painter.setTransform(QTransform().rotate(angle));
                painter.setOpacity(opacity);
                painter.drawImage(label->getVisiableRect(), label->getVisiableImg());
                //img.save(tr("C:\\Users\\Onglu\\Desktop\\test\\Composed_%1x%2_%3").arg(rect1.width()).arg(rect1.height()).arg(file));
                //qDebug() << __FILE__ << __LINE__ << "photo:" << file << opacity;
            }

            index++;
        }
        else
        {
            QVariantMap frame = data["frame"].toMap();
            QSize size = QSize(frame["width"].toInt() * m_ratioSize.width(), frame["height"].toInt() * m_ratioSize.height());
            QPoint pos = QPoint(frame["x"].toInt() * m_ratioSize.width(), frame["y"].toInt() * m_ratioSize.height());
            int x = pos.x() - size.width() / 2;
            int y = pos.y() - size.height() / 2;

            QImage img(size, QImage::Format_ARGB32);
            if (!img.loadFromData(data["picture"].toByteArray()))
            {
                continue;
            }

            painter.setTransform(QTransform().rotate(angle));
            painter.setOpacity(opacity);

            img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter.drawImage(x, y, img);
            //qDebug() << __FILE__ << __LINE__ << "mask:" << file << opacity << angle;
        }
    }

    painter.setTransform(QTransform().rotate(getAngle()));
    painter.setOpacity(getOpacity());
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.drawImage(0, 0, composedImg);
    painter.end();

    if (PhotoLayer::VisiableImgTypeScreen == m_type)
    {
        setPixmap(QPixmap::fromImage(composedImg));
    }
    else
    {
        composedImg.save(fileName);
    }
}

void BgdLayer::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter;
    const int delta = 2;
    QBrush borderColor = m_enter ? Qt::darkCyan : Qt::transparent;

    if (m_borderColor == borderColor)
    {
        return;
    }

    painter.begin(this);
    painter.setPen(QPen(m_borderColor = borderColor, delta));
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter.fillRect(m_visiableRect, Qt::transparent);
    painter.drawRect(m_visiableRect.adjusted(delta, delta, -delta, -delta));
    painter.end();
}
