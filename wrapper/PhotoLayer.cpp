#include "PhotoLayer.h"
#include <QMouseEvent>
#include <QDebug>

bool PhotoLayer::loadPhoto(const QVariantMap &photoLayer,
                           QSizeF ratioSize,
                           QRect bgdRect,
                           const QString &replaced)
{
    int width = 0, height = 0, cx = 0, cy = 0;
    QSize maskSize;
    QPoint maskPos;
    QVariantMap frame, maskLayer;
    QPixmap pix;
    QImage maskImg(maskSize, QImage::Format_ARGB32);
    bool ok = false;

    if (photoLayer.isEmpty())
    {
        goto end;
    }

    m_fileName = photoLayer["filename"].toString();
    if (m_fileName.isEmpty() || !pix.loadFromData(photoLayer["picture"].toByteArray()))
    {
        goto end;
    }

    //m_fileName = replaced.isEmpty() ? photoLayer["filename"].toString() : replaced;

    if (!ratioSize.width())
    {
        ratioSize.setWidth(1);
    }

    if (!ratioSize.height())
    {
        ratioSize.setHeight(1);
    }

    m_ratioSize = ratioSize;
    m_bgdRect = bgdRect;
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_ratioSize << bgdRect << m_fileName;

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

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_picFile << maskFile << maskSize;
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << maskSize << maskPos << bgdRect;

    if (loadPicture(pix, QSize(width, height)) && maskImg.loadFromData(maskLayer["picture"].toByteArray()))
    {
//        setOpacity(m_opacity);
//        rotate(m_angle);

        QPoint topLeft = bgdRect.topLeft();
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << topLeft << parentWidget()->pos();
        topLeft.rx() += cx - width / 2;
        topLeft.ry() += cy - height / 2;
        setGeometry(QRect(topLeft, m_size));
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << geometry();

        m_maskImgs[VISIABLE_IMG_ORIGINAL] = maskImg;
        m_maskImgs[VISIABLE_IMG_SCREEN] = maskImg.scaled(maskSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_maskRect = QRect(maskPos, maskSize);

        m_opacity = photoLayer["opacity"].toReal();
        m_angle = photoLayer["rotation"].toReal();

        updateVisiableRect(m_visiableRects[VisiableRectTypeCopied]);
        ok = true;
    }
    else
    {
end:
        m_visiableImgs[VISIABLE_IMG_SCREEN] = m_visiableImgs[VISIABLE_IMG_ORIGINAL] = QImage();
        m_maskImgs[VISIABLE_IMG_SCREEN] = m_maskImgs[VISIABLE_IMG_ORIGINAL] = QImage();
        m_bgdRect = m_maskRect = QRect(0, 0, 0, 0);
        m_ratioSize = QSizeF(1, 1);
        m_opacity = 1;
        m_angle = 0;
    }

    clear();

    return ok;
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

void PhotoLayer::blend(bool actual)
{
    if (!hasPicture() || (m_maskImgs[VISIABLE_IMG_SCREEN].isNull() && m_maskImgs[VISIABLE_IMG_ORIGINAL].isNull()))
    {
        return;
    }

    QPoint pos = mapTo(parentWidget(), QPoint(0, 0));
    QSize size = this->size();
    QRect picRect = !actual ? QRect(pos, size) : actualRect(pos, size);
    QPixmap pix = !actual ? m_bk : m_ori.scaled(picRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);;
    QImage srcImg = pix.toImage().convertToFormat(QImage::Format_ARGB32);

    pos = m_maskRect.topLeft();
    size = m_maskRect.size();
    QRect maskRect = !actual ? QRect(pos, size) : actualRect(pos, size);
    QImage maskImg;

    if (actual)
    {
        maskImg = m_maskImgs[VISIABLE_IMG_ORIGINAL].scaled(maskRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
    else
    {
        maskImg = m_maskImgs[VISIABLE_IMG_SCREEN];
    }

    //qDebug() << __FILE__ << __LINE__ << picRect << maskRect << maskRect.intersects(picRect)<< maskRect.contains(picRect) << picRect.contains(maskRect) << picRect.intersects(maskRect);

    if (maskRect.intersects(picRect))
    {
        QRect xrect = maskRect.intersected(picRect);
        int maskx, masky, srcx = 0, srcy = 0;

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << xrect;

        for (int y = 0; y < xrect.height(); y++)
        {
            for (int x = 0; x < xrect.width(); x++)
            {
                srcx = x + xrect.x() - picRect.x();
                srcy = y + xrect.y() - picRect.y();
                maskx = x + xrect.x() - maskRect.x();
                masky = y + xrect.y() - maskRect.y();
                QRgb rgb = srcImg.pixel(srcx, srcy);
                int a = qAlpha(maskImg.pixel(maskx, masky));
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

    m_composedImg = srcImg;
}

void PhotoLayer::movePhoto(QPoint offset)
{
    QPoint topLeft = mapTo(parentWidget(), QPoint(0, 0));

    topLeft.rx() += offset.x();
    topLeft.ry() += offset.y();
    move(topLeft);

    m_visiableRects[VisiableRectTypeCanvas].setRect(m_visiableRects[VisiableRectTypeCanvas].x() + offset.x(),
                                                    m_visiableRects[VisiableRectTypeCanvas].y() + offset.y(),
                                                    m_size.width(),
                                                    m_size.height());
    m_moved = true;
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
        qDebug() << __FILE__ << __LINE__ << "invalid rect!";
        m_visiableRects[VisiableRectTypeDefault] = m_visiableRects[VisiableRectTypeCopied] = QRect(0, 0, 0, 0);
        m_visiableImgs[0] = m_visiableImgs[1] = QImage();
        return;
    }

    blend();
    m_visiableRects[VisiableRectTypeCopied].setRect(x, y, width, height);
    m_visiableImgs[VISIABLE_IMG_SCREEN] = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(m_visiableRects[VisiableRectTypeCopied]);
    //m_visiableImgs[VISIABLE_IMG_SCREEN].save(tr("C:\\Users\\Onglu\\Desktop\\test\\Copied_%1").arg(m_fileName));

//    blend(true);
//    m_visiableImgs[VISIABLE_IMG_ORIGINAL] = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(actualRect(m_visiableRects[VisiableRectTypeCopied]));
//    m_visiableImgs[VISIABLE_IMG_ORIGINAL].save(tr("C:\\Users\\Onglu\\Desktop\\test\\Copied_Ori_%1").arg(m_fileName));
//    qDebug() << __FILE__ << __LINE__ << rect << m_visiableImgs[VISIABLE_IMG_ORIGINAL].isNull();

    left = qMax(m_bgdRect.x(), topLeft.x()) - m_bgdRect.x() + x;
    top = qMax(m_bgdRect.y(), topLeft.y()) - m_bgdRect.y() + y;
    m_visiableRects[VisiableRectTypeDefault].setRect(left, top, width, height);

//    qDebug() << __FILE__ << __LINE__
//             //<< m_visiableRects[VisiableRectTypeCanvas]
//             //<< m_visiableRects[VisiableRectTypeFixed]
//             << m_visiableRects[VisiableRectTypeCopied]
//             << m_visiableRects[VisiableRectTypeDefault]
//             ;
}

void PhotoLayer::updateVisiableRect(QRect copiedRect)
{
    QPoint topLeft = mapTo(parentWidget(), QPoint(0, 0));
    //qDebug() << __FILE__ << __LINE__ << topLeft << m_size << m_bgdRect;

    if (copiedRect.isNull())
    {
        /* Form where */
        int x = m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() - topLeft.x() : 0;
        int y = m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() - topLeft.y() : 0;

        /* How big */
        int width = m_bgdRect.width() < m_size.width() - x ? m_bgdRect.width() : m_size.width() - x;
        int height = m_bgdRect.height() < m_size.height() - y ? m_bgdRect.height() : m_size.height() - y;

        blend();
        m_visiableRects[VisiableRectTypeCopied].setRect(x, y, width, height);
        m_visiableImgs[VISIABLE_IMG_SCREEN] = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(m_visiableRects[VisiableRectTypeCopied]);
        //m_visiableImgs[VISIABLE_IMG_SCREEN].save(tr("C:\\Users\\Onglu\\Desktop\\test\\Visiable_%1").arg(m_fileName));

//        blend(true);
//        m_visiableImgs[VISIABLE_IMG_ORIGINAL] = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(actualRect(m_visiableRects[VisiableRectTypeCopied]));
//        m_visiableImgs[VISIABLE_IMG_ORIGINAL].save(tr("C:\\Users\\Onglu\\Desktop\\test\\Visiable_Ori_%1").arg(m_fileName));
//        qDebug() << __FILE__ << __LINE__ << rect << m_visiableImgs[VISIABLE_IMG_ORIGINAL].isNull();

        x = qMax(m_bgdRect.x(), topLeft.x()) - m_bgdRect.x();
        y = qMax(m_bgdRect.y(), topLeft.y()) - m_bgdRect.y();
        m_visiableRects[VisiableRectTypeDefault].setRect(x, y, width, height);
    }

    m_visiableRects[VisiableRectTypeCanvas].setRect(topLeft.x() - m_bgdRect.x(),
                                                    topLeft.y() - m_bgdRect.y(),
                                                    m_size.width(),
                                                    m_size.height());

    if (m_visiableRects[VisiableRectTypeFixed].isNull())
    {
        m_visiableRects[VisiableRectTypeFixed].setRect(qMax(m_bgdRect.x(), m_maskRect.x()) - m_bgdRect.x(),
                                                       qMax(m_bgdRect.y(), m_maskRect.y()) - m_bgdRect.y(),
                                                       m_maskRect.width(),
                                                       m_maskRect.height());
    }

//    qDebug() << __FILE__ << __LINE__
//             //<< m_visiableRects[VisiableRectTypeCanvas]
//             //<< m_visiableRects[VisiableRectTypeFixed]
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
        m_moved = false;
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
