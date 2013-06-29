#include "BgdLayer.h"
#include <QPainter>
#include <QDebug>

void BgdLayer::compose(int type, const QString &fileName)
{
    if (VISIABLE_IMG_TYPES <= type || m_srcImg[type].isNull())
    {
        return;
    }

    QImage composedImg = m_srcImg[type];
    QSizeF ratioSize = VISIABLE_IMG_SCREEN == type ? m_ratioSize : QSizeF(1, 1);

    int index = 0;
    QPainter painter(&composedImg);
    painter.fillRect(composedImg.rect(), Qt::transparent);

    foreach (const QVariant &layer, m_layers)
    {
        qreal opacity = 1, angle = 0;
        QVariantMap data = layer.toMap();
        QString file = data["filename"].toString();

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
                QRect rect = label->visiableRect();
                QImage img = label->visiableImg(type);
                //painter.setTransform(QTransform().rotate(angle));
                painter.setOpacity(opacity);
                painter.drawImage(VISIABLE_IMG_SCREEN == type ? rect : label->actualRect(rect), img);
                //img.save(tr("C:\\Users\\Onglu\\Desktop\\test\\Composed_%1x%2_%3").arg(rect1.width()).arg(rect1.height()).arg(file));
                //qDebug() << __FILE__ << __LINE__ << "photo:" << file << rect1 << type;
            }

            index++;
        }
        else
        {
            QVariantMap frame = data["frame"].toMap();
            QSize size = QSize(frame["width"].toInt() * ratioSize.width(), frame["height"].toInt() * ratioSize.height());
            QPoint pos = QPoint(frame["x"].toInt() * ratioSize.width(), frame["y"].toInt() * ratioSize.height());
            int x = pos.x() - size.width() / 2;
            int y = pos.y() - size.height() / 2;

            QImage img(size, QImage::Format_ARGB32);
            if (!img.loadFromData(data["picture"].toByteArray()))
            {
                continue;
            }

            opacity = data["opacity"].toReal();
            angle = data["angle"].toReal();
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

    if (VISIABLE_IMG_SCREEN == type)
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
