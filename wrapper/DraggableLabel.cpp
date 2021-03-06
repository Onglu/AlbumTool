#include "DraggableLabel.h"
#include "child/PictureChildWidget.h"
#include <QMouseEvent>
#include <QCursor>
#include <QPainter>

DraggableLabel::DraggableLabel(QSize size, const QString &mimeType, QWidget *parent):
    QLabel(parent),
    m_mimeType(mimeType),
    m_bgdColor(Qt::transparent)
{
    resize(size);
    setAlignment(Qt::AlignCenter);

    if (parent)
    {
        setObjectName(parent->objectName());
    }
}

DraggableLabel::DraggableLabel(const QPixmap &pix, QSize size, const QString &mimeType, QWidget *parent):
    QLabel(parent),
    m_mimeType(mimeType)
{
    resize(size);
    setAlignment(Qt::AlignCenter);
    loadPicture(pix, size);
}

void DraggableLabel::loadPicture(const QPixmap &pix, QSize size, qreal angle, Qt::Axis axis)
{
    m_pix = pix;

    if (pix.isNull())
    {
        setStyleSheet("color: rgb(255, 0, 0)");
        setText(tr("无法显示"));    // Bug: doesn't show text in case of owning a parent widget
    }
    else
    {
        if (angle || Qt::ZAxis != axis)
        {
            m_pix = pix.transformed(QTransform().rotate(angle, axis));
        }

        if (size.width() && size.height())
        {
            setPixmap(m_pix.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

QPixmap DraggableLabel::getPicture(bool scaled) const
{
    if (scaled)
    {
        const QPixmap *pix = pixmap();
        if (pix)
        {
            return *pix;
        }
    }

    return m_pix;
}

bool DraggableLabel::hasPicture() const
{
    const QPixmap *pix = pixmap();
    if (!pix || pix->isNull())
    {
        return false;
    }

    return true;
}

inline bool DraggableLabel::meetDragDrop() const
{
    if (!hasPicture() || m_mimeType.isEmpty())
    {
        return false;
    }

    return true;
}

bool DraggableLabel::meetDragDrop(const QString &mimeType) const
{
    if (meetDragDrop() && m_mimeType == mimeType)
    {
        return true;
    }

    return false;
}

void DraggableLabel::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
    {
        emit clicked();

        if (meetDragDrop())
        {
            m_startPos = event->pos();
        }
    }
}

void DraggableLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
    {
        emit dblClicked();
    }
}

void DraggableLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (!Qt::LeftButton & event->buttons() || !meetDragDrop() ||
        qMin(size().width(), size().height()) / 2 > (event->pos() - m_startPos).manhattanLength())
    {
        return;
    }

    setCursor(QCursor(Qt::DragMoveCursor));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QPixmap pix = *pixmap();
    QPoint offset = QPoint(event->pos() - rect().topLeft());
    stream << pix << offset;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(m_mimeType, data);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    QPixmap temp = pix.scaled(this->width() / 1.2, this->height() / 1.2, Qt::KeepAspectRatio);
    QPainter painter;
    painter.begin(&temp);
    painter.fillRect(temp.rect(), QColor(127, 127, 127, 127));
    painter.end();
    drag->setPixmap(temp);
    drag->setHotSpot(QPoint(temp.width()/2, temp.height() / 2));

    if (Qt::CopyAction == drag->exec(Qt::CopyAction | Qt::MoveAction))
    {
        show();
    }
}

void DraggableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && meetDragDrop())
    {
        setCursor(QCursor(Qt::ArrowCursor));
        m_startPos = event->pos();
    }
}

void DraggableLabel::flush()
{
    clear();
    m_belongings.clear();
}

void DraggableLabel::accept(bool inner)
{
    QPixmap pix = getPicture();
    if (pix.isNull())
    {
        return;
    }

    int usedTimes = m_belongings["used_times"].toInt();
    if (!inner)
    {
        m_belongings["used_times"] = ++usedTimes;
    }

    QBrush bgdColor = 0 < usedTimes ? QColor(192, 192, 192, 192) : Qt::transparent;
    if (m_bgdColor != bgdColor)
    {
        QImage img = pix.toImage();
        QPainter painter(&pix);

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(pix.rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(0, 0, img);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.fillRect(img.rect(), m_bgdColor = bgdColor);
        painter.end();

        pix = QPixmap::fromImage(img);
        setPixmap(pix);
    }

    emit hasAccepted(m_belongings);
}
