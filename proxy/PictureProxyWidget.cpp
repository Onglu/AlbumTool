#include "PictureProxyWidget.h"
#include "child/PictureChildWidget.h"
#include "wrapper/PictureGraphicsScene.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

QTransform PictureProxyWidget::m_trans;
QGraphicsScene *PictureProxyWidget::m_focusScene = NULL;

PictureProxyWidget::PictureProxyWidget(PictureChildWidget *child, QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QGraphicsProxyWidget(parent, wFlags),
    m_pChildWidget(child),
    m_detachable(false),
    m_empty(false)
{
    Q_ASSERT(m_pChildWidget);

    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setWidget(m_pChildWidget);
    connect(m_pChildWidget, SIGNAL(itemSelected(/*bool*/)), SLOT(selectItem(/*bool*/)));
    connect(m_pChildWidget, SIGNAL(itemDblSelected()), SLOT(dblSelectItem()));
    connect(m_pChildWidget, SIGNAL(itemUnselected()), SLOT(unselectItem()));
    connect(m_pChildWidget, SIGNAL(itemDetached()), SLOT(detachItem()));

    m_pTimeLine = new QTimeLine(150, this);
    connect(m_pTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(updateStep(qreal)));
    connect(m_pTimeLine, SIGNAL(stateChanged(QTimeLine::State)), this, SLOT(stateChanged(QTimeLine::State)));
}

void PictureProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsProxyWidget::hoverEnterEvent(event);
    scene()->setActiveWindow(this);
    if (m_pTimeLine->currentValue() != 1)
        zoomIn();
}

void PictureProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsProxyWidget::hoverLeaveEvent(event);
    if (m_pTimeLine->direction() != QTimeLine::Backward || m_pTimeLine->currentValue() != 0)
        zoomOut();
}

void PictureProxyWidget::updateStep(qreal step)
{
    QRectF r = boundingRect();
    setTransform(QTransform()
                 .translate(r.width() / 2, r.height() / 2)
                 .scale(1 + 0.1 * step, 1 + 0.1 * step)
                 .translate(-r.width() / 2, -r.height() / 2));
}

void PictureProxyWidget::stateChanged(QTimeLine::State state)
{
    if (state == QTimeLine::Running)
    {
        if (m_pTimeLine->direction() == QTimeLine::Forward)
            setCacheMode(ItemCoordinateCache);
    }
    else if (state == QTimeLine::NotRunning)
    {
        if (m_pTimeLine->direction() == QTimeLine::Backward)
            setCacheMode(DeviceCoordinateCache);
    }

    m_trans = transform();
}

void PictureProxyWidget::zoomIn()
{
    if (m_pTimeLine->direction() != QTimeLine::Forward)
        m_pTimeLine->setDirection(QTimeLine::Forward);

    if (m_pTimeLine->state() == QTimeLine::NotRunning)
        m_pTimeLine->start();
}

void PictureProxyWidget::zoomOut()
{
    if (m_pTimeLine->direction() != QTimeLine::Backward)
        m_pTimeLine->setDirection(QTimeLine::Backward);

    if (m_pTimeLine->state() == QTimeLine::NotRunning)
        m_pTimeLine->start();
}

void PictureProxyWidget::selectItem(/*bool bSingle*/)
{
    if (!scene() || !m_pChildWidget)
    {
        return;
    }

    //qDebug() << __FILE__ << __LINE__ << bSingle;

    bool bSingle = Qt::ControlModifier == QApplication::keyboardModifiers() ? false/* Multi-selection */ : true;
    if (bSingle)
    {
        PictureProxyWidget *proxyWidget = NULL;
        PictureChildWidget *childWidget = NULL;
        QList<QGraphicsItem *> items = scene()->selectedItems();
        int n = items.size();

        if (n)
        {
            foreach (QGraphicsItem *item, items)
            {
                proxyWidget = static_cast<PictureProxyWidget *>(item);
                if (proxyWidget && (childWidget = (PictureChildWidget *)proxyWidget->widget()))
                {
                    childWidget->updateBorder(false);
                    proxyWidget->setSelected(false);
                }
            }

            if (1 == n && this == proxyWidget)
            {
                return; // Does not repeat select the same one
            }
        }
    }

    bool bSelected = isSelected() ? false : true;

    m_pChildWidget->updateBorder(bSelected);
    setSelected(bSelected);

    if (bSelected)
    {
        m_focusScene = scene();
        QCoreApplication::postEvent(scene()->parent(), new QEvent(CustomEvent_Item_Selected));
    }

    //qDebug() << "PhotoProxyWidget::bSingle" << bSingle;
}

void PictureProxyWidget::dblSelectItem()
{
    QWidget *owner = NULL;
    if (!scene() || !(owner = (QWidget *)scene()->parent()) || !m_pChildWidget)
    {
        return;
    }

    m_pChildWidget->updateBorder(true);
    setSelected(true);

    m_focusScene = scene();
    QCoreApplication::postEvent(owner, new QEvent(CustomEvent_Item_Selected));

    ChildWidgetsMap widgetsMap;
    PictureChildWidget *childWidget = NULL;
    PictureProxyWidget *proxyWidget = NULL;
    QList<QGraphicsItem *> items = m_focusScene->items();

    foreach (QGraphicsItem *item, items)
    {
        if ((proxyWidget = static_cast<PictureProxyWidget *>(item)) && (childWidget = &proxyWidget->getChildWidget()))
        {
            widgetsMap.insert(childWidget->getIndex(), childWidget);
        }
    }

    m_pChildWidget->open(widgetsMap);
}

void PictureProxyWidget::unselectItem()
{
    if (!scene() || !m_pChildWidget)
    {
        return;
    }

    m_pChildWidget->updateBorder(false);
    setSelected(false);
}

void PictureProxyWidget::detachItem()
{
    setSelected(true);
    m_detachable = true;
    m_focusScene = scene();
    QCoreApplication::postEvent(scene()->parent(), new QEvent(CustomEvent_Item_Detached));
}

void PictureProxyWidget::clearTimes()
{
    DraggableLabel *picLabel = m_pChildWidget->getPictureLabel();
    if (picLabel)
    {
        QVariantMap &belongings = picLabel->getBelongings();
        if (belongings["used_times"].toInt())
        {
            belongings["used_times"] = 0;
            picLabel->accept(true);
        }
    }
}

bool PictureProxyWidget::excludeItem(const QString &picFile)
{
    DraggableLabel *picLabel = m_pChildWidget->getPictureLabel();

    /* Set the usetimes label text for photo */
    if (picLabel && picFile == picLabel->getPictureFile())
    {
        QVariantMap &belongings = picLabel->getBelongings();
        int usedTimes = belongings["used_times"].toInt();
        if (0 < usedTimes)
        {
            belongings["used_times"] = usedTimes - 1;
            picLabel->accept(true);
            return true;
        }
    }

    return false;
}
