#include "LayerLabel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

const QImage &LayerLabel::loadPicture(const QVariantMap &photoLayer,
                                      QSizeF ratioSize,
                                      QRect bgdRect,
                                      const QString &replaced)
{
    int width = 0, height = 0, cx = 0, cy = 0;
    QSize picSize, maskSize;
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

    if (PictureLabel::loadPicture(m_fileName, QSize(width, height)))
    {
//        setOpacity(m_opacity);
//        rotate(m_angle);

        QPoint topLeft = bgdRect.topLeft();
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this->geometry().topLeft();
        topLeft.rx() += cx - width / 2;
        topLeft.ry() += cy - height / 2;
        setGeometry(QRect(topLeft, m_size));

        QImage maskImg(maskSize, QImage::Format_ARGB32);
        maskImg.load(maskFile);
        m_maskImg = maskImg.scaled(maskSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_maskRect = QRect(maskPos, maskSize);

        blendImg();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this->geometry() << bgdRect << this->minimumSize();

        updateView();

        clear();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << m_visiableRect[VISIABLE_RECT_TYPE_CANVAS] << m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER];
    }

    return m_visiableImg;
}

void LayerLabel::changePicture(const QVariantMap &belongings)
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

void LayerLabel::blendImg()
{
    if (m_bk.isNull() || m_maskImg.isNull())
    {
        return;
    }

    QImage srcImg = m_bk.toImage().convertToFormat(QImage::Format_ARGB32);

    QRect picRect(mapTo(parentWidget(), QPoint(0, 0)), m_bk.size());
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << maskRect.intersects(picRect)<< maskRect.contains(picRect) << picRect.contains(maskRect)<< maskRect << picRect << xrect;

    if (m_maskRect.intersects(picRect))
    {
        QRect xrect = m_maskRect.intersected(picRect);
        int maskx, masky, srcx = 0, srcy = 0;

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

inline void LayerLabel::updateView()
{
    QPoint topLeft = mapTo(parentWidget(), QPoint(0, 0));
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << topLeft << picSize;

    /* Form where */
    int x = m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() - topLeft.x() : 0;
    int y = m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() - topLeft.y() : 0;

    /* How big */
    QSize picSize = m_bk.size();
    int width = m_bgdRect.width() < picSize.width() - x ? m_bgdRect.width() : picSize.width() - x;
    int height = m_bgdRect.height() < picSize.height() - y ? m_bgdRect.height() : picSize.height() - y;

    m_copiedRect.setRect(x, y, width, height);
    m_visiableImg = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(m_copiedRect);

    m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER].setRect(m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() : topLeft.x(),
                                                         m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() : topLeft.y(),
                                                         width,
                                                         height);

    topLeft = m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER].topLeft();
    m_visiableRect[VISIABLE_RECT_TYPE_CANVAS].setRect(topLeft.x() - m_bgdRect.x(),
                                                      topLeft.y() - m_bgdRect.y(),
                                                      width,
                                                      height);

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER] << m_visiableRect[VISIABLE_RECT_TYPE_CANVAS];
}

void LayerLabel::setMoveable(bool moveable)
{
    m_moveable = moveable;

    if (moveable)
    {
        //setWindowFlags(m_flags | Qt::WindowStaysOnTopHint);
        setPixmap(m_bk);
        setCursor(Qt::ClosedHandCursor);
    }
    else
    {
        //setWindowFlags(m_flags);
        clear();
        setCursor(Qt::OpenHandCursor);
    }

    repaint();
}

void LayerLabel::mousePressEvent(QMouseEvent *event)
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

void LayerLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter;
    const int delta = 2;
    QBrush borderColor = m_moveable ? Qt::darkCyan : Qt::transparent;

    if (m_borderColor == borderColor)
    {
        return;
    }

    painter.begin(this);
    painter.setPen(QPen(m_borderColor = borderColor, delta));
    painter.setRenderHints(QPainter::Antialiasing);
    painter.drawRect(m_copiedRect.adjusted(delta, delta, -delta, -delta));
    painter.end();
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_copiedRect;
}
