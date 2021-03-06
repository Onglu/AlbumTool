#include "PictureChildWidget.h"
#include <QDragEnterEvent>
#include <QPainter>
#include <QDebug>
#include "page/TaskPageWidget.h"

PictureChildWidget::PictureChildWidget(const QString &file, TaskPageWidget *container) :
    QWidget(0),
    m_container(container)
{
    QString name;
    setToolTip(Converter::getFileName(file, name, true));
}

PictureChildWidget::PictureChildWidget(const QString &file, QSize fixedSize, bool droppable, TaskPageWidget *container) :
    QWidget(0),
    m_container(container),
    m_index(0),
    m_indexLabel(NULL),
    m_picLabel(NULL),
    m_borderColor(Qt::transparent)
{
    Q_ASSERT(m_container);

    m_defaultPalette = palette();

    QString name;
    setToolTip(Converter::getFileName(file, name, true));

    setFixedSize(fixedSize);
    setAcceptDrops(droppable);
}

void PictureChildWidget::setIndex(int index)
{
    if (0 < index && m_index != index && m_indexLabel)
    {
        m_records.insert("index", index);
        m_indexLabel->setText(tr("%1").arg(index));
        m_index = index;
    }
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

void PictureChildWidget::setPictureLabel(const QPixmap &pix,
                                         QSize scaledSize,
                                         const QString &mimeType,
                                         QWidget *parent,
                                         QPoint pos)
{
    if (!acceptDrops() || !parent || m_picLabel)
    {
        return;
    }

    m_picLabel = new DraggableLabel(pix, scaledSize, mimeType, parent);
    connect(m_picLabel, SIGNAL(hasAccepted(QVariantMap)), SLOT(onAccept(QVariantMap)));
    connect(m_picLabel, SIGNAL(clicked()), SIGNAL(itemSelected()));
    connect(m_picLabel, SIGNAL(dblClicked()), SIGNAL(itemDblSelected()));

    if (QPoint(0, 0) != pos)
    {
        m_picLabel->move(pos);
    }
}

void PictureChildWidget::onAccept(const QVariantMap &belongings)
{
    QPalette pal;

    if (belongings["used_times"].toInt())
    {
        pal.setColor(QPalette::Window, Qt::lightGray);
    }
    else
    {
        pal = m_defaultPalette;
    }

    setPalette(pal);

    QString name;
    if (this->toolTip() != Converter::getFileName(m_picLabel->getPictureFile(), name, true))
    {
        setToolTip(name);
    }
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
            if ((picLabel = childWidget->getPictureLabel()) &&
                    "" != (picture = picLabel->getPictureFile()))
            {
                pictures << QDir::toNativeSeparators(picture);
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
    //qDebug() << __FILE__ << __LINE__ << "dragger: from =" << from << ", to =" << to;

    dragger.setBelongings(to);
    m_picLabel->setBelongings(from);

    dragger.setPixmap(m_picLabel->getPicture());
    dragger.accept(true);

    QString name;
    setToolTip(Converter::getFileName(m_picLabel->getPictureFile(), name, true));
}

bool PictureChildWidget::meetDragDrop(QDropEvent *event)
{
    if (m_picLabel && event->mimeData()->hasFormat(m_picLabel->getMimeType()))
    {
        return true;
    }

    event->ignore();

    return false;
}

void PictureChildWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (meetDragDrop(event))
    {
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

        //qDebug() << __FILE__ << __LINE__ << picLabel->getPictureFile() << m_picLabel->getPictureFile();

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

        m_container->noticeChanged();
    }
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
