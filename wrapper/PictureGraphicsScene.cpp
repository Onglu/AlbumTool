#include "PictureGraphicsScene.h"
#include "child/AlbumChildWidget.h"
#include "page/EditPageWidget.h"
#include <QtCore/qmath.h>
#include <QDebug>
#include <QGraphicsView>

PictureGraphicsScene::PictureGraphicsScene(const QBrush &bkclr,
                                           LayoutMode layout,
                                           SceneType type,
                                           QGraphicsView *view,
                                           QObject *parent) :
    QGraphicsScene(parent), m_layout(layout), m_type(type), m_loadFromDisk(false), m_pView(view)
{
    Q_ASSERT(SceneType_End > type);
    Q_ASSERT(m_pView);

    setBackgroundBrush(bkclr);

    if (parent)
    {
        installEventFilter(parent);
    }

    m_pView->setScene(this);
}

void PictureGraphicsScene::insertProxyWidget(int index,
                                             PictureProxyWidget *proxyWidget,
                                             QString file)
{
    if (0 >= index || !proxyWidget)
    {
        return;
    }

    addItem(proxyWidget);

    if (!file.isEmpty())
    {
        m_filesList.append(file);
    }

    m_proxyWidgetsMap.insert(index, proxyWidget);
}

inline int PictureGraphicsScene::getViewWidth() const
{
    if (LayoutMode_Horizontality == m_layout)
    {
        return (m_proxyWidgetsMap.size() * items().last()->boundingRect().width());
    }

    return m_pView->width();
}

void PictureGraphicsScene::adjustItemPos()
{
    const int count = m_proxyWidgetsMap.size();
    if (!count)
    {
        return;
    }

    QGraphicsItem *item = items().last();
    QRectF rect = item->boundingRect();
    const int cx = getViewWidth() / rect.width();    // The number of items on per x-axis
    int ix = 0;                                 // The index of items on per x-axis
    int iy = 0;                                 // The index of items on per y-axis
    int index = count - 1;
    int row = 0;

    iy = qFloor((qreal)index / cx);
    if (0 == index % cx)
    {
        ix = 0;
    }
    else
    {
        row = qCeil((qreal)index / cx);
        ix = index - cx * (row - 1);
    }

    qreal left = ix * rect.width();
    qreal top = iy * rect.height();

//    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "viewWidth:" << viewWidth << ", itemWidth:" << rect.width()
//             << "count:" << count << ", index:" << index << ", ix: " << ix << ", iy: " << iy << left << top;

    m_proxyWidgetsMap[count]->setPos(left, top);
    setSceneRect(itemsBoundingRect());
}

void PictureGraphicsScene::adjustViewLayout(int viewWidth)
{
    const GraphicsItemsList itemsList = m2l();
    const int count = itemsList.size();
    if (!count)
    {
        return;
    }

    if (!viewWidth)
    {
        viewWidth = getViewWidth();
    }

    QGraphicsItem *item = itemsList.first();
    QRectF rect = item->boundingRect();
    const int cx = viewWidth / rect.width();    // The number of items on per x-axis
    int ix = 0;                                 // The index of items on per x-axis
    int iy = 0;                                 // The index of items on per y-axis
    int index = 0;

//    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "viewSize:" << viewSize << ", itemSize:" << rect.size()
//             << "count:" << count << ", from:" << from << ", cx: " << cx << ", iy: " << iy;

    while (index < count)
    {
        int row = 0;

        if (cx > index)
        {
            ix = index;
        }
        else if (cx == index)
        {
            ix = 0;
            if (LayoutMode_Horizontality != m_layout)
            {
                iy = 1;
            }
        }
        else
        {
            row = qCeil((qreal)index / cx);
            ix = index - cx * (row - 1);
        }

        qreal left = ix * rect.width();
        qreal top = iy * rect.height();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_layout << iy << top;

        if (LayoutMode_Grid == m_layout && viewWidth < left + rect.width())
        {
            left = 0;
            top += rect.height();
            iy++;
        }

//        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "row:" << row << ", ix:" << ix
//                 << ", iy:" << iy << ", left:" << left << ", top:" << top << m_layout;

        item = itemsList.at(index);
        item->setPos(left, top);
        index++;
    }

    setSceneRect(itemsBoundingRect());
}

const GraphicsItemsList &PictureGraphicsScene::m2l()
{
    m_itemsList.clear();

    foreach (PictureProxyWidget *proxyWidget, m_proxyWidgetsMap)
    {
        m_itemsList.append(proxyWidget);
    }

    return m_itemsList;
}

const ProxyWidgetsMap &PictureGraphicsScene::l2m()
{
    int index = 0;
    ProxyWidgetsMap tmpMap;
    GraphicsItemsList itemsList = items();

    foreach (QGraphicsItem *item, itemsList)
    {
        PictureProxyWidget *proxyWidget = static_cast<PictureProxyWidget *>(item);
        if (proxyWidget)
        {
            tmpMap.insert(proxyWidget->getChildWidget().getIndex(), proxyWidget);
        }
    }

    m_proxyWidgetsMap.clear();
    foreach (PictureProxyWidget *proxyWidget, tmpMap)
    {
        proxyWidget->getChildWidget().setIndex(++index);
        m_proxyWidgetsMap.insert(index, proxyWidget);
    }

    return m_proxyWidgetsMap;
}

void PictureGraphicsScene::removeProxyWidget(PictureProxyWidget *proxyWidget)
{
    if (proxyWidget)
    {
        removeItem(proxyWidget);
        delete proxyWidget;
        proxyWidget = NULL;
        l2m();
    }
}

void PictureGraphicsScene::removeProxyWidgets(bool all, EditPageWidget *pEditPage)
{
    QGraphicsItem *item = NULL;
    GraphicsItemsList itemsList = all ? items() : selectedItems();

    while (!itemsList.isEmpty())
    {
        if ((item = (QGraphicsItem *)itemsList.first()))
        {
            PictureProxyWidget *proxyWidget = static_cast<PictureProxyWidget *>(item);
            if (proxyWidget)
            {
                PictureChildWidget &childWidget = proxyWidget->getChildWidget();
                DraggableLabel *picLabel = NULL;
                QString picFile;

                /***
                 * If delete a photo or a template from the photos scene or the templates scene, we should update
                 * the album views and thumb views at the same time.
                 */
                if (SceneType_Albums > m_type && (picLabel = childWidget.getPictureLabel()))
                {
                    bool empty = false;
                    ProxyWidgetsMap &albumProxyWidgets = m_scensVector.at(SceneType_Albums)->getProxyWidgetsMap();

                    picFile = picLabel->getPictureFile();

                    foreach (PictureProxyWidget *albumProxyWidget, albumProxyWidgets)
                    {
                        albumProxyWidget->excludeItem(picFile);
                        empty = albumProxyWidget->isEmpty();
                    }

                    if (empty) // Clear all ?
                    {
                        ProxyWidgetsMap &photoProxyWidgets = m_scensVector.at(SceneType_Photos)->getProxyWidgetsMap();
                        foreach (PictureProxyWidget *photoProxyWidget, photoProxyWidgets)
                        {
                            photoProxyWidget->clearTimes();
                        }

                        ProxyWidgetsMap &tmplProxyWidgets = m_scensVector.at(SceneType_Templates)->getProxyWidgetsMap();
                        foreach (PictureProxyWidget *tmplProxyWidget, tmplProxyWidgets)
                        {
                            tmplProxyWidget->clearTimes();
                        }
                    }

                    if (pEditPage)
                    {
                        pEditPage->removeThumbs(picFile);
                    }

                    m_filesList.removeOne(picFile);
                }

                /***
                 * If delete a photo from a albums scene or a thumbs scene, we should update the view of the
                 * corresponding photo item.
                 */
                QStringList photosList, tmplsList;

                if (SceneType_Albums == m_type)
                {
                    AlbumChildWidget &albumWidget = static_cast<AlbumChildWidget &>(childWidget);
                    photosList = albumWidget.getPhotosList();
                    tmplsList.append(albumWidget.getTmplLabel().getPictureFile());
                }

                if (SceneType_Thumbs == m_type && (picLabel = childWidget.getPictureLabel()) && !all/* Not is clear */)
                {
                    photosList.append(picLabel->getPictureFile());
                }

                if (photosList.size() || tmplsList.size())
                {
                    excludeItems(SceneType_Photos, photosList);
                    excludeItems(SceneType_Templates, tmplsList);
                }

                m_proxyWidgetsMap.remove(childWidget.getIndex());
            }

            removeItem(item);
            delete item;
            item = NULL;
            itemsList.removeFirst();
        }
    }

    l2m();
}

void PictureGraphicsScene::excludeItems(SceneType type, const QStringList &filesList)
{
    if (SceneType_Templates >= type)
    {
        foreach (const QString &file, filesList)
        {
            ProxyWidgetsMap &proxyWidgets = m_scensVector.at(type)->getProxyWidgetsMap();
            foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
            {
                if (proxyWidget->excludeItem(file))
                {
                    break;
                }
            }
        }
    }
}

void PictureGraphicsScene::clearFocusSelection(bool all)
{
    PictureProxyWidget *proxyWidget = NULL;
    GraphicsItemsList itemsList = all ? items() : selectedItems();

    if (itemsList.size())
    {
        foreach (QGraphicsItem *item, itemsList)
        {
            if ((proxyWidget = static_cast<PictureProxyWidget *>(item)))
            {
                proxyWidget->getChildWidget().updateBorder(false);
            }
        }

        clearFocus();
        clearSelection();
    }
}

void PictureGraphicsScene::checkChanges(QVariantList &changesList)
{
    PictureProxyWidget *proxyWidget = NULL;
    GraphicsItemsList itemsList = items();

    foreach (QGraphicsItem *item, itemsList)
    {
        if ((proxyWidget = static_cast<PictureProxyWidget *>(item)))
        {
            changesList << proxyWidget->getChildWidget().getChanges();
        }
    }
}
