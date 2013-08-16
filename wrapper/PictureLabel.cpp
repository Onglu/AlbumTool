#include "PictureLabel.h"
#include "utility.h"
#include <QtCore/qmath.h>
#include <QPainter>
#include <QDebug>

bool PictureLabel::loadPicture(const QPixmap &pix, QSize size)
{
    bool ret = pix.isNull();

    m_default = size;
    m_ori = pix;

    if (ret)
    {
        m_size = m_scaled = size;
    }
    else
    {
        m_bk = m_ori.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setPixmap(m_bk);
        m_size = m_scaled = m_bk.size();
    }

    setFixedSize(m_size);

    return !ret;
}

void PictureLabel::setOpacity(QPixmap &pix, qreal opacity) const
{
    if (pix.isNull() || 0 > opacity || 1 < opacity)
    {
        return;
    }

    QPixmap result(pix.size());
    result.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&result);
    painter.setOpacity(opacity);
    painter.drawPixmap(0, 0, pix);
    painter.end();
    pix = result;
}

void PictureLabel::rotate(qreal angle, Qt::Axis axis)
{
    if (!m_ori.isNull() && m_angle != angle)
    {
        //m_angle = Converter::rotation(m_angle, angle);

        QTransform trans;
        trans.translate(m_ori.width() / 2, m_ori.height() / 2).rotate(angle, axis);
        m_ori = m_ori.transformed(trans, Qt::SmoothTransformation);

        //m_ori = m_ori.transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation);

        int width = m_scaled.width(), height = m_scaled.height();

//        static bool rotated = false;

//        if (!rotated)
//        {
//            rotated = true;
//            QRect rect = this->geometry();
//            qDebug() << __FILE__ << __LINE__ << m_scaled << rect << rect.center();
//        }

        if (Qt::YAxis == axis && 180 == angle && m_angle)
        {
            m_angle *= -1;
        }

        if (Qt::ZAxis == axis)
        {
            m_angle = angle;
            m_axis = axis;

            if (angle)
            {
                if (0 > angle)
                {
                    angle *= -1;
                }

                if (90 < angle)
                {
                    angle = 180 - angle;
                }

                qreal arc = angle * 3.1415926535 / 180;
                int sw = qCos(arc) * width + qSin(arc) * height;
                int sh = qCos(arc) * height + qSin(arc) * width;
                width = sw;
                height = sh;
            }
            else
            {
                width = m_default.width() + 2 * MARGIN_SPACE;
                height = m_default.height() + 2 * MARGIN_SPACE;
            }
        }

        m_bk = m_ori.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_size = m_bk.size();

        QRect old = this->geometry();
        //qDebug() << __FILE__ << __LINE__ << "before:" << m_size << m_angle << width << height << old.center();
        setFixedSize(m_size);

        QRect curr = this->geometry();
        curr.moveCenter(old.center());
        setGeometry(curr);
        setPixmap(m_bk);
        //qDebug() << __FILE__ << __LINE__ << "after:" << curr << this->geometry().center();
    }
    else
    {
        m_angle = 0;
    }
}

void PictureLabel::scale(float ratio, Qt::AspectRatioMode aspectRatioMode)
{
    if (m_bk.isNull() || 2 <= ratio)
    {
        return;
    }

    if (ratio)
    {
        m_bk = m_ori.scaled(1 == ratio ? m_default : QSize(m_bk.width() * ratio, m_bk.height() * ratio),
                            aspectRatioMode,
                            Qt::SmoothTransformation);
    }
    else
    {
        m_bk = m_ori;
    }

    m_size = m_scaled = m_bk.size();
    setFixedSize(m_size);
    setPixmap(m_bk);
}
