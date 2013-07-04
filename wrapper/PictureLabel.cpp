#include "PictureLabel.h"
#include "utility.h"
#include <QPainter>
#include <QDebug>

bool PictureLabel::loadPicture(const QPixmap &pix, QSize size)
{
    m_ori = pix;
    if (m_ori.isNull())
    {
        m_size = size;
        return false;
    }

    m_bk = m_ori.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_size = m_default = m_bk.size();

    setFixedSize(m_size);
    setPixmap(m_bk);

    return true;
}

void PictureLabel::scaledZoom(float scale, Qt::AspectRatioMode aspectRatioMode)
{
    if (m_bk.isNull() || 2 <= scale)
    {
        return;
    }

    if (scale)
    {
        m_bk = m_ori.scaled(1 == scale ? m_default : QSize(m_bk.width() * scale, m_bk.height() * scale),
                            aspectRatioMode,
                            Qt::SmoothTransformation);
    }
    else
    {
        m_bk = m_ori;
    }

    m_size = m_bk.size();
    setFixedSize(m_size);
    setPixmap(m_bk);
}

void PictureLabel::setOpacity(QPixmap &pix, qreal opacity)
{
    if (pix.isNull() || 0 > opacity || 1 < opacity)
    {
        return;
    }

    QPixmap result(pix.size());
    result.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&result);
    painter.setOpacity(m_opacity = opacity);
    painter.drawPixmap(0, 0, pix);
    painter.end();
    pix = result;
}

void PictureLabel::rotate(qreal angle, Qt::Axis axis)
{
    if (!m_bk.isNull())
    {
        m_angle = Converter::rotation(m_angle, angle);
        m_axis = axis;
        m_ori = m_ori.transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation);
        m_bk = m_bk.transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation);
        setFixedSize(m_bk.size());
        setPixmap(m_bk);
    }
}
