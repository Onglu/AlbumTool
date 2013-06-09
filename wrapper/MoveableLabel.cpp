#include "MoveableLabel.h"

MoveableLabel(const QString &fileName, QSize size, QWidget *parent) : QLabel(parent)
{
    loadPicture(fileName, size);
}

void MoveableLabel::setMoveable(bool moveable)
{
    if (moveable)
    {
        setCursor(Qt::ClosedHandCursor);
    }
    else
    {
        setCursor(Qt::OpenHandCursor);
    }

    m_moveable = moveable;
}

void MoveableLabel::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton & event->buttons() && !m_ori.isNull() && !m_moveable)
    {
        setMoveable(true);
        QPoint pos = mapTo(parentWidget(), QPoint(0, 0));
        pos.rx() += event->pos().x();
        pos.ry() += event->pos().y();
        emit clicked(*this, pos);
    }
}


