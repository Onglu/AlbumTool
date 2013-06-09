#ifndef MOVEABLELABEL_H
#define MOVEABLELABEL_H

#include <QLabel>
#include <QMouseEvent>

class MoveableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MoveableLabel(QWidget *parent = 0) : QLabel(parent), m_moveable(false){}

    void setMoveable(bool moveable)
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

    bool isMoveable(void) const {return m_moveable;}

signals:
    void clicked(MoveableLabel &self, QPoint pos);

protected:
    void mousePressEvent(QMouseEvent *event)
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

private:
    bool m_moveable;
};

#endif // MOVEABLELABEL_H
