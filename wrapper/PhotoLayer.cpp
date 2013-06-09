#include "PhotoLayer.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

const QImage &PhotoLayer::loadPhoto(const QVariantMap &photoLayer,
                                    QSizeF ratioSize,
                                    QRect bgdRect,
                                    const QString &replaced)
{
    int width = 0, height = 0, cx = 0, cy = 0;
    QSize maskSize;
    QPoint maskPos;
    QString maskFile;
    QVariantMap frame, maskLayer;

    if (photoLayer.isEmpty() || m_fileName == photoLayer["filename"].toString() ||
        (!replaced.isEmpty() && m_fileName == replaced))
    {
        return m_visiableImg;
    }

    m_fileName = replaced.isEmpty() ? photoLayer["filename"].toString() : replaced;
    m_ratioSize = ratioSize;
    m_bgdRect = bgdRect;

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_ratioSize << bgdRect << m_picFile;

    frame = photoLayer["frame"].toMap();
    width = frame["width"].toInt() * ratioSize.width();
    height = frame["height"].toInt() * ratioSize.height();
    cx = frame["x"].toInt() * ratioSize.width();
    cy = frame["y"].toInt() * ratioSize.height();
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << cx << cy << width << height;

    maskLayer = photoLayer["maskLayer"].toMap();
    frame = maskLayer["frame"].toMap();
    maskSize = QSize(frame["width"].toInt() * ratioSize.width(), frame["height"].toInt() * ratioSize.height());
    maskPos.rx() = bgdRect.x() + frame["x"].toInt() * ratioSize.width() - maskSize.width() / 2;
    maskPos.ry() = bgdRect.y() + frame["y"].toInt() * ratioSize.height() - maskSize.height() / 2;
    maskFile = maskLayer["filename"].toString();

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_picFile << maskFile << maskSize;
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << maskSize << maskPos << bgdRect;

    m_opacity = photoLayer["opacity"].toReal();
    m_angle = photoLayer["rotation"].toReal();

    if (loadPicture(m_fileName, QSize(width, height)))
    {
//        setOpacity(m_opacity);
//        rotate(m_angle);

        QPoint topLeft = bgdRect.topLeft();
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << topLeft << parentWidget()->pos();
        topLeft.rx() += cx - width / 2;
        topLeft.ry() += cy - height / 2;
        setGeometry(QRect(topLeft, m_size));
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << geometry();

        QImage maskImg(maskSize, QImage::Format_ARGB32);
        maskImg.load(maskFile);
        m_maskImg = maskImg.scaled(maskSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_maskRect = QRect(maskPos, maskSize);

        updateVisiableRect();
        clear();
    }

    return m_visiableImg;
}

void PhotoLayer::changePhoto(const QVariantMap &belongings)
{
//    QString picFile = belongings["picture_file"].toString();
//    if (m_picFile == picFile)
//    {
//        return;
//    }

//    m_picFile = picFile;
//    m_ori = QPixmap(m_picFile);
//    if (m_ori.isNull())
//    {
//        return;
//    }

//    qreal angle = belongings["rotation_angle"].toReal();
//    Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
//    m_bk = m_ori.scaled(m_bk.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    rotate(ui->pictureLabel, m_bk, angle, axis);

//    blend();
//    updateView();

//    emit hasUpdated();
}

void PhotoLayer::blend()
{
    if (m_bk.isNull() || m_maskImg.isNull())
    {
        return;
    }

    QImage srcImg = m_bk.toImage().convertToFormat(QImage::Format_ARGB32);
    QRect picRect(mapTo(parentWidget(), QPoint(0, 0)), m_bk.size());
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_maskRect.intersects(picRect)<< m_maskRect.contains(picRect) << picRect.contains(m_maskRect)<< m_maskRect << picRect << m_maskRect.intersected(picRect);

    if (m_maskRect.intersects(picRect))
    {
        QRect xrect = m_maskRect.intersected(picRect);
        int maskx, masky, srcx = 0, srcy = 0;

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << xrect;

        for (int y = 0; y < xrect.height(); y++)
        {
            for (int x = 0; x < xrect.width(); x++)
            {
                srcx = x + xrect.x() - picRect.x();
                srcy = y + xrect.y() - picRect.y();
                maskx = x + xrect.x() - m_maskRect.x();
                masky = y + xrect.y() - m_maskRect.y();
                QRgb rgb = srcImg.pixel(srcx, srcy);
                int a = qAlpha(m_maskImg.pixel(maskx, masky));
                srcImg.setPixel(srcx, srcy, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), a));
            }
        }

        int cx = xrect.x() - picRect.x();
        int cy = xrect.y() - picRect.y();

        if (0 < cx && 0 < cy)
        {
            for (int y = 0; y < picRect.height(); y++)
            {
                for (int x = 0; x < cx; x++)
                {
                    srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }

            for (int y = 0; y < cy; y++)
            {
                for (int x = 0; x < picRect.width(); x++)
                {
                    srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }
        }

        for (int y = 0; y < picRect.height(); y++)
        {
            for (int x = srcx; x < picRect.width(); x++)
            {
                srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }

        for (int y = srcy; y < picRect.height(); y++)
        {
            for (int x = 0; x < picRect.width(); x++)
            {
                srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }
    }

//    if (picRect.contains(maskRect))
//    {

//    }

    m_composedImg = srcImg;
}

void PhotoLayer::movePhoto(QPoint offset)
{
    QPoint topLeft = mapTo(parentWidget(), QPoint(0, 0));

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "move:" << topLeft << offset;
    topLeft.rx() += offset.x();
    topLeft.ry() += offset.y();
    move(topLeft);

    m_visiableRects[VisiableRectTypeCanvas].setRect(m_visiableRects[VisiableRectTypeCanvas].x() + offset.x(),
                                                    m_visiableRects[VisiableRectTypeCanvas].y() + offset.y(),
                                                    m_size.width(),
                                                    m_size.height());

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "move:" << topLeft
//             //<< m_visiableRects[VisiableRectTypeContainer]
             //<< m_visiableRects[VisiableRectTypeCanvas]
             //<< m_visiableRects[VisiableRectTypeFixed]
//             << m_visiableRects[VisiableRectTypeCopied]
             //<< m_visiableRects[VisiableRectTypeDefault];
}

void PhotoLayer::updateCopiedRect()
{
    int from, left, top, x = 0, y = 0, width = 0, height = 0;
    QPoint topLeft = mapTo(parentWidget(), QPoint(0, 0));

    left = topLeft.x() - m_bgdRect.x();
    top = topLeft.y() - m_bgdRect.y();

    if (m_visiableRects[VisiableRectTypeFixed].x() > left + m_visiableRects[VisiableRectTypeFixed].width() ||
        m_visiableRects[VisiableRectTypeFixed].right() < left)
    {
        x = width = 0;
    }
    else
    {
        from = qAbs(m_visiableRects[VisiableRectTypeFixed].x() - left);
        x = m_visiableRects[VisiableRectTypeFixed].x() > left ? from : 0;
        width = m_visiableRects[VisiableRectTypeFixed].width() - from;
    }

    if (m_visiableRects[VisiableRectTypeFixed].y() > top + m_visiableRects[VisiableRectTypeFixed].height() ||
        m_visiableRects[VisiableRectTypeFixed].bottom() < top)
    {
        y = height = 0;
    }
    else
    {
        from = qAbs(m_visiableRects[VisiableRectTypeFixed].y() - top);
        y = m_visiableRects[VisiableRectTypeFixed].y() > top ? from : 0;
        height = m_visiableRects[VisiableRectTypeFixed].height() - from;
    }

    if ((!x && !width) || (!y && !height))
    {
        //qDebug() << __FILE__ << __LINE__ << "invalid rect!";
        m_visiableRects[VisiableRectTypeDefault] = m_visiableRects[VisiableRectTypeCopied] = QRect(0, 0, 0, 0);
        m_visiableImg = QImage();
        return;
    }

    blend();
    m_visiableRects[VisiableRectTypeCopied].setRect(x, y, width, height);
    m_visiableImg = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(m_visiableRects[VisiableRectTypeCopied]);

    left = qMax(m_bgdRect.x(), topLeft.x()) - m_bgdRect.x() + x;
    top = qMax(m_bgdRect.y(), topLeft.y()) - m_bgdRect.y() + y;
    m_visiableRects[VisiableRectTypeDefault].setRect(left, top, width, height);

//    qDebug() << __FILE__ << __LINE__ << "now:" << x << y << width << height << m_maskRect
//             //<< m_visiableRects[VisiableRectTypeContainer]
//             //<< m_visiableRects[VisiableRectTypeCanvas]
//             << m_visiableRects[VisiableRectTypeFixed]
//             << m_visiableRects[VisiableRectTypeCopied]
//             << m_visiableRects[VisiableRectTypeDefault]
//             ;
}

void PhotoLayer::updateVisiableRect()
{
    QPoint topLeft = mapTo(parentWidget(), QPoint(0, 0));
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << topLeft << m_size << m_bgdRect;

    /* Form where */
    int x = m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() - topLeft.x() : 0;
    int y = m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() - topLeft.y() : 0;

    /* How big */
    int width = m_bgdRect.width() < m_size.width() - x ? m_bgdRect.width() : m_size.width() - x;
    int height = m_bgdRect.height() < m_size.height() - y ? m_bgdRect.height() : m_size.height() - y;

    blend();
    m_visiableRects[VisiableRectTypeCopied].setRect(x, y, width, height);
    m_visiableImg = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(m_visiableRects[VisiableRectTypeCopied]);

//    m_visiableRects[VisiableRectTypeContainer].setRect(m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() : topLeft.x(),
//                                                       m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() : topLeft.y(),
//                                                       width,
//                                                       height);

    m_visiableRects[VisiableRectTypeCanvas].setRect(topLeft.x() - m_bgdRect.x(),
                                                    topLeft.y() - m_bgdRect.y(),
                                                    m_size.width(),
                                                    m_size.height());

    x = qMax(m_bgdRect.x(), topLeft.x()) - m_bgdRect.x();
    y = qMax(m_bgdRect.y(), topLeft.y()) - m_bgdRect.y();
    m_visiableRects[VisiableRectTypeDefault].setRect(x, y, width, height);

    if (m_visiableRects[VisiableRectTypeFixed].isNull())
    {
        m_visiableRects[VisiableRectTypeFixed].setRect(qMax(m_bgdRect.x(), m_maskRect.x()) - m_bgdRect.x(),
                                                       qMax(m_bgdRect.y(), m_maskRect.y()) - m_bgdRect.y(),
                                                       m_maskRect.width(),
                                                       m_maskRect.height());
    }

//    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << m_maskRect
////             //<< m_visiableRects[VisiableRectTypeContainer]
//             << m_visiableRects[VisiableRectTypeCanvas]
//             << m_visiableRects[VisiableRectTypeFixed]
//             << m_visiableRects[VisiableRectTypeCopied]
//             << m_visiableRects[VisiableRectTypeDefault]
//                ;
}

void PhotoLayer::setMoveable(bool moveable)
{
    m_moveable = moveable;

    if (moveable)
    {
        setCursor(Qt::ClosedHandCursor);
        QPixmap pix = m_bk;
        setOpacity(pix, 0.6);
    }
    else
    {
        setCursor(Qt::OpenHandCursor);
        updateRect();
    }
}

void PhotoLayer::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton & event->buttons() && !m_moveable)
    {
        setMoveable(true);
        QPoint pos = mapTo(parentWidget(), QPoint(0, 0));
        pos.rx() += event->pos().x();
        pos.ry() += event->pos().y();
        emit clicked(*this, pos);
    }
}

void BgdLayer::compose(const QVariantList &layers, const QVector<PhotoLayer *> labels)
{
    if (m_bgdImg.isNull())
    {
        return;
    }

    m_composedImg = m_bgdImg;

    int index = 0;
    QPainter painter(&m_composedImg);
    painter.fillRect(m_composedImg.rect(), Qt::transparent);

    foreach (const QVariant &layer, layers)
    {
        qreal opacity = 1, angle = 0;
        QVariantMap data = layer.toMap();
        QString filename = data["filename"].toString();
        int type = data["type"].toInt();

        if (1 == type)
        {
            PhotoLayer *label = labels.at(index);
            if (label && label->hasPicture())
            {
                //m_photoWidgets[index]->getShape(opacity, angle);
                //painter.setTransform(QTransform().rotate(angle));
                painter.setOpacity(opacity);
                painter.drawImage(label->visiableRect(/*PhotoLayer::VisiableRectTypeCanvas*/), label->visiableImg());
                //qDebug() << __FILE__ << __LINE__ << "photo:" << index << label->visiableRect();
                index++;
            }
        }
        else
        {
            QVariantMap frame = data["frame"].toMap();
            QSize size = QSize(frame["width"].toInt() * m_ratioSize.width(), frame["height"].toInt() * m_ratioSize.height());
            QPoint pos = QPoint(frame["x"].toInt() * m_ratioSize.width(), frame["y"].toInt() * m_ratioSize.height());
            int x = pos.x() - size.width() / 2;
            int y = pos.y() - size.height() / 2;

            QImage img(size, QImage::Format_ARGB32_Premultiplied);
            img.load(filename);
            img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            opacity = data["opacity"].toReal();
            angle = data["angle"].toReal();

            painter.setTransform(QTransform().rotate(angle));
            painter.setOpacity(opacity);
            painter.drawImage(x, y, img);
            //qDebug() << __FILE__ << __LINE__ << "mask:" << filename;
        }
    }

    painter.setTransform(QTransform().rotate(m_angle));
    painter.setOpacity(m_opacity);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.drawImage(0, 0, m_composedImg);
    painter.end();

    setPixmap(QPixmap::fromImage(m_composedImg));
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
