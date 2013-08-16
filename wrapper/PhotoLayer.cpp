#include "PhotoLayer.h"
#include "DraggableLabel.h"
#include <QMouseEvent>
#include <QDebug>
#include "defines.h"

PhotoLayer *PhotoLayer::m_replaced = NULL;

PhotoLayer::PhotoLayer(VisiableImgType type, QWidget *parent) : PictureLabel(parent),
    m_type(type),
    m_moveable(false),
    m_moved(false),
    m_ratioSize(QSizeF(1, 1)),
    m_bgdRect(QRect(0, 0, 0, 0)),
    m_thumbId(0)
{
//    setFrameShape(QFrame::Box);
//    setFrameShadow(QFrame::Raised);
    setAlignment(Qt::AlignCenter);
}

bool PhotoLayer::loadPhoto(bool restore,
                           const QVariantMap &photoLayer,
                           const QString &photoFile,
                           qreal angle,
                           Qt::Axis axis,
                           QRect location,
                           //const QPixmap &mask,
                           int thumbId
                           )
{
    int width = 0, height = 0, cx = 0, cy = 0;
    QPoint topLeft = m_bgdRect.topLeft();
    QSize maskSize;
    QPoint maskPos;
    QVariantMap frame, maskLayer;
    QPixmap pix;
    QImage maskImg(maskSize, QImage::Format_ARGB32);
    bool ret, ok = false;

    if (photoLayer.isEmpty())
    {
        goto end;
    }

    m_fileName = photoLayer["filename"].toString();
    if (m_fileName.isEmpty())
    {
        goto end;
    }

    m_picFile = photoFile;

    //qDebug() << __FILE__ << __LINE__ << m_ratioSize << m_bgdRect << m_fileName << photoFile << location;
    if (!photoFile.isEmpty() && m_src.load(photoFile))
    {
        if (angle || Qt::ZAxis != axis)
        {
            m_src = m_src.transformed(QTransform().rotate(angle, axis));
        }

        pix = m_src;
        m_angle = photoLayer["rotation"].toReal();
        m_axis = Qt::ZAxis;

        if (m_angle || Qt::ZAxis != m_axis)
        {
            pix = pix.transformed(QTransform().rotate(m_angle, m_axis));
        }
    }
    else
    {
        //pix.loadFromData(photoLayer["picture"].toByteArray()); // for test
        //goto end;
        //restore = false;
    }

    frame = photoLayer["frame"].toMap();
    width = frame["width"].toInt() * m_ratioSize.width();
    height = frame["height"].toInt() * m_ratioSize.height();
    cx = frame["x"].toInt() * m_ratioSize.width();
    cy = frame["y"].toInt() * m_ratioSize.height();
    //qDebug() << __FILE__ << __LINE__ << frame << cx << cy << width << height;

    //qDebug() << __FILE__ << __LINE__ << m_ratioSize << m_bgdRect << m_fileName << photoFile << m_src.isNull();
    ret = loadPicture(pix, QSize(width, height));
    topLeft.rx() += cx - width / 2;
    topLeft.ry() += cy - height / 2;
    move(topLeft);

    m_opacity = photoLayer["opacity"].toFloat();
    if (ret)
    {
        setOpacity(m_ori, m_opacity);
        setOpacity(m_bk, m_opacity);
        //qDebug() << __FILE__ << __LINE__ << m_picFile << frame << cx << cy << width << height << topLeft << m_size << m_angle << m_axis;
    }

    if (0 < thumbId)
    {
        m_thumbId = thumbId;
    }

    maskLayer = photoLayer["maskLayer"].toMap();
    if (!maskLayer.isEmpty() && maskImg.loadFromData(maskLayer["picture"].toByteArray()))
    //if (!mask.isNull())
    {
        frame = maskLayer["frame"].toMap();
        maskSize = QSize(frame["width"].toInt() * m_ratioSize.width(), frame["height"].toInt() * m_ratioSize.height());
        maskPos.rx() = m_bgdRect.x() + frame["x"].toInt() * m_ratioSize.width() - maskSize.width() / 2;
        maskPos.ry() = m_bgdRect.y() + frame["y"].toInt() * m_ratioSize.height() - maskSize.height() / 2;
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << maskSize << maskPos << m_bgdRect;

        //m_maskImg = mask.toImage().convertToFormat(QImage::Format_ARGB32).scaled(maskSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_maskImg = maskImg.scaled(maskSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_maskRect = QRect(maskPos, maskSize);

        int x = qMax(m_bgdRect.x(), m_maskRect.x()) - m_bgdRect.x();
        int y = qMax(m_bgdRect.y(), m_maskRect.y()) - m_bgdRect.y();
        width = m_bgdRect.width() < m_maskRect.width() ? m_bgdRect.width() : m_maskRect.width();
        height = m_bgdRect.height() < m_maskRect.height() ? m_bgdRect.height() : m_maskRect.height();
        m_visiableRects[VisiableRectTypeFixed].setRect(x, y, width, height);
    }
    else
    {
        //qDebug() << __FILE__ << __LINE__ << "none mask layer!";
        m_maskImg = QImage();
        m_maskRect = QRect();

        if (location.isNull())
        {
            qDebug() << __FILE__ << __LINE__ << "display rect is null";
            goto end;
        }
        else
        {
            m_visiableRects[VisiableRectTypeFixed] = location;
        }

        //qDebug() << __FILE__ << __LINE__ << m_fileName << m_picFile << m_visiableRects[VisiableRectTypeFixed];
    }

    updateCanvasRect(restore);

    if (!m_ori.isNull())
    {
        updateVisiableRect();
    }

    clear();
    setAcceptDrops(ok = true);

end:
    if (!ok)
    {
        flush();
    }

    return ok;
}

void PhotoLayer::changePhoto(const QString &photoFile, qreal angle, Qt::Axis axis)
{
    m_picFile = photoFile;
    m_src = QPixmap(photoFile);
    if (!m_src.isNull())
    {
        if (angle || Qt::ZAxis != axis)
        {
            m_src = m_src.transformed(QTransform().rotate(angle, axis));
        }

        m_ori = m_src;
        if (m_angle || Qt::ZAxis != m_axis)
        {
            m_ori = m_ori.transformed(QTransform().rotate(m_angle, m_axis));
        }

        if (1 != m_opacity)
        {
            setOpacity(m_ori, m_opacity);
        }

        clear();
        updateCanvasRect();
        updateRect();

        //qDebug() << __FILE__ << __LINE__ << file << angle << axis << m_size;
    }
    else
    {
        clear();
    }
}

void PhotoLayer::blend()
{
    if (m_ori.isNull() || m_visiableRects[VisiableRectTypeCopied].isNull())
    {
        m_visiableImg = QImage();
        return;
    }

    QRect picRect = this->geometry();
    QPixmap pix = m_ori.scaled(m_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);;
    m_composedImg = pix.toImage().convertToFormat(QImage::Format_ARGB32);

    //qDebug() << __FILE__ << __LINE__ << picRect << m_maskRect << m_maskRect.intersects(picRect)<< m_maskRect.contains(picRect) << picRect.contains(m_maskRect) << picRect.intersects(m_maskRect);

    if (!m_maskImg.isNull() && m_maskRect.intersects(picRect))
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
                QRgb rgb = m_composedImg.pixel(srcx, srcy);
                int a = qAlpha(m_maskImg.pixel(maskx, masky));
                m_composedImg.setPixel(srcx, srcy, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), a));
            }
        }

        const int cx = xrect.x() - picRect.x();
        const int cy = xrect.y() - picRect.y();
        const int width = picRect.width();
        const int height = picRect.height();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << cx << cy;

        if (0 < cx && 0 < cy)
        {
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < cx; x++)
                {
                    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << height;
                    m_composedImg.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }

            for (int y = 0; y < cy; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << width;
                    m_composedImg.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }
        }

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << srcx << srcy << height << width;

        for (int y = 0; y < height; y++)
        {
            for (int x = srcx; x < width; x++)
            {
                //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << height << width;
                m_composedImg.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }

        for (int y = srcy; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << height << width;
                m_composedImg.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }
    }

    m_visiableImg = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(m_visiableRects[VisiableRectTypeCopied]);
}

QString PhotoLayer::getLayerId() const
{
    return m_fileName.left(m_fileName.length() - strlen(PIC_FMT));
}

void PhotoLayer::movePhoto(QPoint offset)
{
    QPoint topLeft = this->geometry().topLeft();
    topLeft.rx() += offset.x();
    topLeft.ry() += offset.y();
    move(topLeft);

    m_visiableRects[VisiableRectTypeCanvas].setRect(m_visiableRects[VisiableRectTypeCanvas].x() + offset.x(),
                                                    m_visiableRects[VisiableRectTypeCanvas].y() + offset.y(),
                                                    m_size.width(),
                                                    m_size.height());

    m_moved = true;
}

void PhotoLayer::updateCanvasRect(bool restore)
{
    QPoint topLeft = this->geometry().topLeft();

    if (restore)
    {
        m_visiableRects[VisiableRectTypeCanvas].setRect(topLeft.x() - m_bgdRect.x(),
                                                        topLeft.y() - m_bgdRect.y(),
                                                        m_size.width(),
                                                        m_size.height());
        return;
    }

    if (m_ori.isNull())
    {
        return;
    }

    float ratio = 1;
    bool portrait = m_ori.width() < m_ori.height() ? true : false;
    int width = m_visiableRects[VisiableRectTypeFixed].width() + 2 * MARGIN_SPACE;
    int height = m_visiableRects[VisiableRectTypeFixed].height() + 2 * MARGIN_SPACE;

    if (portrait)
    {
        ratio = m_ori.width() > width ? m_ori.width() / width : width / m_ori.width();
        m_bk = m_ori.scaled(width, m_ori.height() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        ratio = m_ori.height() > height ? m_ori.height() / height : height / m_ori.height();
        m_bk = m_ori.scaled(m_ori.width() * ratio, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (!this->pixmap())
    {
        setPixmap(m_bk);
    }

    m_size = m_scaled = m_bk.size();
    setFixedSize(m_size);

    move(m_bgdRect.x() + m_visiableRects[VisiableRectTypeFixed].x(),
         m_bgdRect.y() + m_visiableRects[VisiableRectTypeFixed].y());

    topLeft = this->geometry().topLeft();
    topLeft.rx() -= MARGIN_SPACE;
    topLeft.ry() -= MARGIN_SPACE;

    if (!portrait)
    {
        topLeft.rx() -= (m_bk.width() - width) / 2;
    }
    move(topLeft);

    m_visiableRects[VisiableRectTypeCanvas].setRect(topLeft.x() - m_bgdRect.x(),
                                                    topLeft.y() - m_bgdRect.y(),
                                                    m_size.width(),
                                                    m_size.height());
}

void PhotoLayer::updateCopiedRect()
{
    int from, left, top, x = 0, y = 0, width = 0, height = 0;
    const QPoint topLeft = this->geometry().topLeft();

    left = topLeft.x() - m_bgdRect.x();
    top = topLeft.y() - m_bgdRect.y();
    //qDebug() << __FILE__ << __LINE__ << m_size << topLeft << m_bgdRect << left << top;

    if (m_visiableRects[VisiableRectTypeFixed].x() > left + m_size.width() ||
        m_visiableRects[VisiableRectTypeFixed].right() < left)
    {
        x = width = 0;
    }
    else
    {
        from = qAbs(m_visiableRects[VisiableRectTypeFixed].x() - left);
        x = m_visiableRects[VisiableRectTypeFixed].x() > left ? from : 0;
        if (!x)
        {
            if (m_visiableRects[VisiableRectTypeFixed].width() < m_size.width())
            {
                width = m_visiableRects[VisiableRectTypeFixed].width() - from;
            }
            else
            {
                width = m_visiableRects[VisiableRectTypeCanvas].right() > m_visiableRects[VisiableRectTypeFixed].right() ? m_size.width() - (m_visiableRects[VisiableRectTypeCanvas].right() - m_visiableRects[VisiableRectTypeFixed].right()) : m_size.width();
            }
        }
        else
        {
            width = m_size.width() - from;
            if (m_visiableRects[VisiableRectTypeFixed].width() < width)
            {
                width = m_visiableRects[VisiableRectTypeFixed].width();
            }
        }

        //qDebug() << __FILE__ << __LINE__ << left << from << x << width;
    }

    if (m_visiableRects[VisiableRectTypeFixed].y() > top + m_size.height() ||
        m_visiableRects[VisiableRectTypeFixed].bottom() < top)
    {
        y = height = 0;
    }
    else
    {
        from = qAbs(m_visiableRects[VisiableRectTypeFixed].y() - top);
        y = m_visiableRects[VisiableRectTypeFixed].y() > top ? from : 0;

        if (!y)
        {
            if (m_visiableRects[VisiableRectTypeFixed].height() < m_size.height())
            {
                height = m_visiableRects[VisiableRectTypeFixed].height() - from;
            }
            else
            {
                height = m_visiableRects[VisiableRectTypeCanvas].bottom() > m_visiableRects[VisiableRectTypeFixed].bottom() ? m_size.height() - (m_visiableRects[VisiableRectTypeCanvas].bottom() - m_visiableRects[VisiableRectTypeFixed].bottom()) : m_size.height();
            }
        }
        else
        {
            height = m_size.height() - from;
            if (m_visiableRects[VisiableRectTypeFixed].height() < height)
            {
                height = m_visiableRects[VisiableRectTypeFixed].height();
            }
        }

        //qDebug() << __FILE__ << __LINE__ << top << from << y << height;
    }

    if ((!x && !width) || (!y && !height))
    {
        qDebug() << __FILE__ << __LINE__ << "invalid rect:" << x << y << width << height;
        m_visiableRects[VisiableRectTypeDefault] = QRect(0, 0, 0, 0);
        m_visiableImg = QImage();
        return;
    }

    m_visiableRects[VisiableRectTypeCopied].setRect(x, y, width, height);
    m_visiableRects[VisiableRectTypeDefault].setRect(left + x, top + y, width, height);

    blend();

//    QString filename = m_fileName;
//    filename.replace("jpg", "png");
//    m_visiableImg.save(tr("C:\\snapshot\\Copied_%1_%2").arg(m_type).arg(filename));

//    qDebug() << __FILE__ << __LINE__ << m_fileName << this->geometry()
//             << m_visiableRects[VisiableRectTypeCanvas]
//             << m_visiableRects[VisiableRectTypeFixed]
//             << m_visiableRects[VisiableRectTypeCopied]
//             << m_visiableRects[VisiableRectTypeDefault]
//             ;
}

void PhotoLayer::updateVisiableRect()
{
    int x, y, width, height;
    const QPoint topLeft = this->geometry().topLeft();
    //qDebug() << __FILE__ << __LINE__ << this->geometry() << m_size;

    if (m_visiableRects[VisiableRectTypeFixed].left() > m_visiableRects[VisiableRectTypeCanvas].right() ||
        m_visiableRects[VisiableRectTypeFixed].top() > m_visiableRects[VisiableRectTypeCanvas].bottom() ||
        m_visiableRects[VisiableRectTypeFixed].right() < m_visiableRects[VisiableRectTypeCanvas].left() ||
        m_visiableRects[VisiableRectTypeFixed].bottom() < m_visiableRects[VisiableRectTypeCanvas].top())
    {
        qDebug() << __FILE__ << __LINE__ << "invalid rect:" << m_visiableRects[VisiableRectTypeFixed] << m_visiableRects[VisiableRectTypeCanvas];
        m_visiableRects[VisiableRectTypeDefault] = QRect(0, 0, 0, 0);
        m_visiableImg = QImage();
        return;
    }

    /* Copy position */
    if (m_visiableRects[VisiableRectTypeCanvas].x() < m_visiableRects[VisiableRectTypeFixed].x())
    {
        x = m_visiableRects[VisiableRectTypeFixed].x() - m_visiableRects[VisiableRectTypeCanvas].x();
    }
    else
    {
        x = m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() - topLeft.x() : 0;
    }

    if (m_visiableRects[VisiableRectTypeCanvas].y() < m_visiableRects[VisiableRectTypeFixed].y())
    {
        y = m_visiableRects[VisiableRectTypeFixed].y() - m_visiableRects[VisiableRectTypeCanvas].y();
    }
    else
    {
        y = m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() - topLeft.y() : 0;
    }

    /* Copy size */
    width = m_size.width() - x;
    if (m_visiableRects[VisiableRectTypeCanvas].right() > m_visiableRects[VisiableRectTypeFixed].right())
    {
        width -= m_visiableRects[VisiableRectTypeCanvas].right() - m_visiableRects[VisiableRectTypeFixed].right();
    }

    height = m_size.height() - y;
    if (m_visiableRects[VisiableRectTypeCanvas].bottom() > m_visiableRects[VisiableRectTypeFixed].bottom())
    {
        height -= m_visiableRects[VisiableRectTypeCanvas].bottom() - m_visiableRects[VisiableRectTypeFixed].bottom();
    }

    m_visiableRects[VisiableRectTypeCopied].setRect(x, y, width, height);

    /* Calculates the drawing rectangle */
    if (0 > m_visiableRects[VisiableRectTypeCanvas].x())
    {
        x = m_visiableRects[VisiableRectTypeFixed].x();
    }
    else
    {
        x += qMax(m_bgdRect.x(), topLeft.x()) - m_bgdRect.x();
    }

    if (0 > m_visiableRects[VisiableRectTypeCanvas].y())
    {
        y = m_visiableRects[VisiableRectTypeFixed].y();
    }
    else
    {
        y += qMax(m_bgdRect.y(), topLeft.y()) - m_bgdRect.y();
    }

    m_visiableRects[VisiableRectTypeDefault].setRect(x, y, width, height);

    blend();

//    QString filename = m_fileName;
//    filename.replace("jpg", "png");
//    m_visiableImg.save(tr("C:\\snapshot\\Visiable_%1_%2").arg(m_type).arg(filename));

//    qDebug() << __FILE__ << __LINE__ << m_fileName << this->geometry()
//             << m_visiableRects[VisiableRectTypeCanvas]
//             << m_visiableRects[VisiableRectTypeFixed]
//             << m_visiableRects[VisiableRectTypeCopied]
//             << m_visiableRects[VisiableRectTypeDefault]
//             ;
}

void PhotoLayer::updateRect()
{
    if (m_ori.isNull())
    {
        return;
    }

    clear();

    //setFixedSize(m_size);

    if (m_size.width() > m_bgdRect.width() || m_size.height() > m_bgdRect.height())
    {
        updateVisiableRect();
    }
    else
    {
        updateCopiedRect();
    }
}

void PhotoLayer::flush(bool all)
{
    if (all)
    {
        for (int i = 0; i < VISIABLE_RECT_TYPES; i++)
        {
            m_visiableRects[i] = QRect(0, 0, 0, 0);
        }

        m_bgdRect = m_maskRect = QRect(0, 0, 0, 0);
        m_visiableImg = m_maskImg = QImage();
        m_ratioSize = QSizeF(1, 1);
        m_opacity = 1;
        m_angle = 0;
        m_axis = Qt::ZAxis;
        //m_picFile = m_fileName = QString();
        setGeometry(0, 0, 0, 0);
    }
    else
    {
        //m_picFile = QString();
        m_visiableImg = QImage();
        m_visiableRects[VisiableRectTypeDefault] = m_visiableRects[VisiableRectTypeCopied] = QRect(0, 0, 0, 0);
        m_size = m_visiableRects[VisiableRectTypeFixed].size();
        setFixedSize(m_size);
        move(m_bgdRect.x() + m_visiableRects[VisiableRectTypeFixed].x(),
             m_bgdRect.y() + m_visiableRects[VisiableRectTypeFixed].y());
        //qDebug() << __FILE__ << __LINE__ << m_picFile << m_size << this->geometry() << m_visiableRects[VisiableRectTypeFixed];
    }

    m_src = m_ori = m_bk = QPixmap();
    m_picFile = m_fileName = QString();
    m_thumbId = 0;

    clear();
}

void PhotoLayer::setMoveable(bool moveable)
{
    m_moveable = moveable;

    if (moveable)
    {
        setCursor(Qt::OpenHandCursor);
        QPixmap pix = m_bk;
        setOpacity(pix, 0.6);
        setPixmap(pix);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        if (m_moved)
        {
            updateRect();
            m_moved = false;
        }
        else
        {
            clear();
        }
    }
}

bool PhotoLayer::zoomAction(float ratio)
{
    int x, y, width = m_bk.width(), height = m_bk.height();
    if (5 > width * ratio || 5 > height * ratio)
    {
        return false;
    }

    scale(ratio, Qt::KeepAspectRatio);
    x = (width - m_bk.width()) / 2;
    y = (height - m_bk.height()) / 2;
    move(pos().x() + x, pos().y() + y);

    m_visiableRects[VisiableRectTypeCanvas].setRect(m_visiableRects[VisiableRectTypeCanvas].x() + x,
                                                    m_visiableRects[VisiableRectTypeCanvas].y() + y,
                                                    m_size.width(),
                                                    m_size.height());

    return true;
}

void PhotoLayer::rotateAction(qreal angle, Qt::Axis axis)
{
    if (Qt::YAxis == axis)
    {
        rotate(180, axis);
        m_src = m_src.transformed(QTransform().rotate(180, axis), Qt::SmoothTransformation);
        m_src.save(m_picFile);
    }
    else
    {
        if (m_angle != angle)
        {
            m_ori = m_src;
            rotate(angle);
            updateCanvasRect(true);
        }
    }
}

void PhotoLayer::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton & event->buttons() && !m_moveable)
    {
        QPoint wpos, epos, pos = event->pos();

//        qDebug() << __FILE__ << __LINE__ << m_picFile
//                 << m_visiableRects[VisiableRectTypeCanvas]
//                 << m_visiableRects[VisiableRectTypeFixed]
//                 << pos;

        wpos = this->geometry().topLeft();
        wpos.rx() += pos.x();
        wpos.ry() += pos.y();

        epos = m_visiableRects[VisiableRectTypeCanvas].topLeft();
        epos.rx() += pos.x();
        epos.ry() += pos.y();

        emit clicked(wpos, epos);
    }
}

void PhotoLayer::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_PHOTO) && !children().contains(event->source()))
    {
        event->acceptProposedAction();
    }
}

void PhotoLayer::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_PHOTO) && !children().contains(event->source()))
    {
        event->acceptProposedAction();
    }
}

void PhotoLayer::dropEvent(QDropEvent *event)
{
    QString currPic = m_picFile;
    DraggableLabel *picLabel = static_cast<DraggableLabel *>(event->source());

    //qDebug() << __FILE__ << __LINE__ << currPic << picLabel->getPictureFile();

    if (picLabel && picLabel->meetDragDrop(DRAGGABLE_PHOTO) && !children().contains(picLabel)
        && picLabel->hasPicture() && currPic != picLabel->getPictureFile())
    {
        QVariantMap belongings = picLabel->getBelongings();
        QString picFile = belongings["picture_file"].toString();
        qreal angle = belongings["rotation_angle"].toReal();
        Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();

        event->acceptProposedAction();

        changePhoto(picFile, angle, axis);

        m_replaced = this;

        //qDebug() << __FILE__ << __LINE__ << m_picFile << m_size << angle << axis;
        //qDebug() << __FILE__ << __LINE__ << currPic << picFile << getLayerId();

        emit replaced(currPic, picFile);
    }
}
