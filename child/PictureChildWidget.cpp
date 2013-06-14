#include "PictureChildWidget.h"
#include <QDragEnterEvent>
#include <QPainter>
#include <QDebug>
#include "page/TaskPageWidget.h"

PictureChildWidget::PictureChildWidget(QSize fixedSize, bool droppable, TaskPageWidget *container) :
    QWidget(0),
    m_container(container),
    m_index(0),
    m_indexLabel(NULL),
    m_picLabel(NULL),
    m_dragging(false),
    m_dropped(false),
    m_borderColor(Qt::transparent)
{
    Q_ASSERT(m_container);

    m_defaultPalette = palette();

    setFixedSize(fixedSize);
    setAcceptDrops(droppable);
}

void PictureChildWidget::setIndexLabel(int index, QLabel *lable, QPoint pos)
{
    if (!m_indexLabel)
    {
        if (lable)
        {
            m_indexLabel = lable;
        }
        else
        {
            m_indexLabel = new QLabel(this);
            m_indexLabel->setFixedSize(61, 16);
            m_indexLabel->move(pos);

            QFont font = m_indexLabel->font();
            font.setBold(true);
            m_indexLabel->setFont(font);
        }
    }

    setIndex(index);
}

void PictureChildWidget::setIndex(int index)
{
    if (0 < index && m_index != index && m_indexLabel)
    {
        m_picAttrMap.insert("index", index);
        m_indexLabel->setText(tr("%1").arg(index));
        m_index = index;
    }
}

void PictureChildWidget::setPictureLabel(/*const QString &picFile*/ const QPixmap &pix,
                                         QSize scaledSize,
                                         const QString &mimeType,
                                         QWidget *parent,
                                         QPoint pos)
{
    if (!acceptDrops() || !parent || m_picLabel)
    {
        return;
    }

    m_picLabel = new DraggableLabel(/*picFile*/ pix, scaledSize, mimeType, parent);
    m_picLabel->installEventFilter(this);
    connect(m_picLabel, SIGNAL(hasAccepted(int)), SLOT(onAccept(int)));

    if (QPoint(0, 0) != pos)
    {
        m_picLabel->move(pos);
    }
}

void PictureChildWidget::onAccept(int usedTimes)
{
    QPalette pal;

    if (usedTimes)
    {
        pal.setColor(QPalette::Window, Qt::lightGray);
    }
    else
    {
        pal = m_defaultPalette;
    }

    setPalette(pal);
}

void PictureChildWidget::open(ChildWidgetsMap &widgetsMap)
{
    QString picture;
    QStringList pictures;
    DraggableLabel *picLabel = NULL;
    TaskPageWidget *container = static_cast<TaskPageWidget *>(m_container);

    if (container && !widgetsMap.isEmpty())
    {
        foreach (PictureChildWidget *childWidget, widgetsMap)
        {
            if ((picLabel = childWidget->getPictureLabel()) && "" != (picture = picLabel->getPictureFile()))
            {
                pictures << picture;
            }
        }

        container->onPreview(pictures, m_index - 1);
    }
}

void PictureChildWidget::swap(DraggableLabel &dragger)
{
    QVariantMap from, to;

    from = dragger.getBelongings();
    to = m_picLabel->getBelongings();
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "dragger: from =" << from << ", to =" << to;

    dragger.setBelongings(to);
    m_picLabel->setBelongings(from);

    dragger.setPixmap(m_picLabel->getPicture());
    dragger.accept(true);
}

bool PictureChildWidget::meetDragDrop(QDropEvent *event)
{
    if (m_picLabel && event->mimeData()->hasFormat(m_picLabel->getMimeType()))
    {
        return true;
    }

    m_dragging = false;
    event->ignore();

    return false;
}

void PictureChildWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (meetDragDrop(event))
    {
        m_dragging = true;

        if (children().contains(event->source()))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
}

void PictureChildWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (meetDragDrop(event))
    {
        m_dropped = false;

        if (children().contains(event->source()))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
}

void PictureChildWidget::dropEvent(QDropEvent *event)
{
    DraggableLabel *picLabel = static_cast<DraggableLabel *>(event->source());
    if (meetDragDrop(event) && picLabel)
    {
        QByteArray data = event->mimeData()->data(m_picLabel->getMimeType());
        QDataStream stream(&data, QIODevice::ReadOnly);
        QPixmap pix;
        QPoint offset;

        //qDebug() << __FILE__ << __LINE__ << event->mimeData()->text() << ", m_picLabel:" << m_picLabel->getPictureFile();

        if (children().contains(picLabel))
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            swap(*picLabel);
            m_picLabel->accept(true);
            event->acceptProposedAction();
        }

        stream >> pix >> offset;
        m_picLabel->setPixmap(pix);

        m_dragging = false;
        m_dropped = true;
        m_container->noticeChanged();
    }
}

bool PictureChildWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (m_picLabel == watched)
    {
        if (QEvent::MouseButtonPress == event->type())
        {
            return clickPicture();
        }
        else if (QEvent::MouseButtonDblClick == event->type())
        {
            return dblClickPicture();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void PictureChildWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    const int delta = 2;
    QRect rect = event->rect();

    painter.begin(this);
    painter.setPen(QPen(m_borderColor, delta));
    painter.setRenderHints(QPainter::Antialiasing);
    painter.drawRect(rect.adjusted(delta, delta, -delta, -delta));
    painter.end();
}
