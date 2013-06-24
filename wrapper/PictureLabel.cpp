#include "PictureLabel.h"
#include "utility.h"
#include <QPainter>

PictureLabel::PictureLabel(QWidget *parent) : QLabel(parent), m_angle(0), m_opacity(1)
{
}

bool PictureLabel::loadPicture(/*const QString &fileName*/ const QPixmap &pix, QSize size)
{
    m_ori = /*QPixmap(m_fileName = fileName)*/ pix;
    if (m_ori.isNull())
    {
        m_size = size;
        return false;
    }

    m_bk = m_ori.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_size = m_bk.size();
    setFixedSize(m_size);
    setPixmap(m_bk);

    return true;
}

void PictureLabel::scaledZoom(float scale)
{
    if (m_bk.isNull() || 2 <= scale)
    {
        return;
    }

    if (!scale)
    {
        m_bk = m_ori;
    }
    else
    {
        m_bk = m_ori.scaled(1 == scale ? m_optimal : QSize(m_bk.width() * scale, m_bk.height() * scale),
                            Qt::KeepAspectRatio,
                            Qt::SmoothTransformation);
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

    QPixmap result(m_size);
    result.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&result);
    painter.setOpacity(m_opacity = opacity);
    painter.drawPixmap(0, 0, pix);
    painter.end();

    setPixmap(pix = result);
}

void PictureLabel::rotate(qreal angle, Qt::Axis axis)
{
    if (!m_bk.isNull())
    {
        m_angle = Converter::rotation(m_angle, angle);
        m_bk = m_bk.transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation);
        setFixedSize(m_bk.size());
        setPixmap(m_bk);
    }
}
