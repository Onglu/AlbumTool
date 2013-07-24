#include "PictureGraphicsScene.h"
#include "child/AlbumChildWidget.h"
#include "child/TemplateChildWidget.h"
#include "page/EditPageWidget.h"
#include "defines.h"
#include <QtCore/qmath.h>
#include <QDebug>
#include <QGraphicsView>
#include <QMessageBox>

PictureGraphicsScene::PictureGraphicsScene(const QBrush &bkclr,
                                           LayoutMode layout,
                                           SceneType type,
                                           QGraphicsView *view,
                                           QObject *parent) :
    QGraphicsScene(parent),
    m_layout(layout),
    m_type(type),
    m_loadFinished(false),
    m_pView(view)
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

void PictureGraphicsScene::addProxyWidget(int index, PictureProxyWidget *proxyWidget)
{
    if (0 >= index || !proxyWidget)
    {
        return;
    }

    addItem(proxyWidget);
    proxyWidget->getChildWidget().setIndex(index);
    m_resultsWidgets.insert(index, proxyWidget);
}

void PictureGraphicsScene::addProxyWidgets(const ProxyWidgetsMap &proxyWidgets)
{
    PictureProxyWidget *proxyWidget = NULL;
    ProxyWidgetsMap::const_iterator iter = proxyWidgets.constBegin();
    while (iter != proxyWidgets.constEnd())
    {
        if ((proxyWidget = static_cast<PictureProxyWidget *>(iter.value())))
        {
            addProxyWidget(proxyWidget->getChildWidget().getIndex(), proxyWidget);
            adjustItemPos();
        }

        ++iter;
    }
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

    m_proxyWidgets.insert(index, proxyWidget);

    if (SceneType_Templates == m_type)
    {
        m_resultsWidgets.insert(index, proxyWidget);
    }
}

PictureProxyWidget *PictureGraphicsScene::getProxyWidget(const ProxyWidgetsMap &proxyWidgets, const QString &picFile)
{
    PictureChildWidget *childWidgt = NULL;

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        if ((childWidgt = proxyWidget->getChildWidgetPtr()) && picFile == childWidgt->getPictureLabel()->getPictureFile())
        {
            return proxyWidget;
        }
    }

    return NULL;
}

inline int PictureGraphicsScene::getViewWidth() const
{
    if (LayoutMode_Horizontality == m_layout)
    {
        return (m_proxyWidgets.size() * items().last()->boundingRect().width());
    }

    return m_pView->width();
}

void PictureGraphicsScene::adjustItemPos(bool partial)
{
    const int count = items().size();
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

    if (partial && SceneType_Templates == m_type)
    {
        m_resultsWidgets[count]->setPos(left, top);
    }
    else
    {
        m_proxyWidgets[count]->setPos(left, top);
    }

    setSceneRect(itemsBoundingRect());
}

void PictureGraphicsScene::adjustViewLayout(int viewWidth)
{
    const GraphicsItemsList itemsList = SceneType_Templates != m_type ? m2l(m_proxyWidgets) : m2l(m_resultsWidgets);
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

const GraphicsItemsList &PictureGraphicsScene::m2l(const ProxyWidgetsMap &proxyWidgets)
{
    m_itemsList.clear();

    foreach (PictureProxyWidget *proxyWidget, proxyWidgets)
    {
        m_itemsList.append(proxyWidget);
    }

    return m_itemsList;
}

const ProxyWidgetsMap &PictureGraphicsScene::l2m(ProxyWidgetsMap &proxyWidgets)
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

    proxyWidgets.clear();
    foreach (PictureProxyWidget *proxyWidget, tmpMap)
    {
        proxyWidget->getChildWidget().setIndex(++index);
        proxyWidgets.insert(index, proxyWidget);
    }

    return proxyWidgets;
}

void PictureGraphicsScene::removeProxyWidget(PictureProxyWidget *proxyWidget)
{
    if (proxyWidget)
    {
        removeItem(proxyWidget);

        if (SceneType_Templates == m_type)
        {
            int index = 1;

            foreach (PictureProxyWidget *resultWidget, m_resultsWidgets)
            {
                if (proxyWidget == resultWidget)
                {
                    m_resultsWidgets.remove(index);
                    l2m(m_resultsWidgets);
                    break;
                }

                index++;
            }
        }

        delete proxyWidget;
        proxyWidget = NULL;
        l2m(m_proxyWidgets);
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
                DraggableLabel *picLabel = childWidget.getPictureLabel();
                QString picFile;

                if (picLabel)
                {
                    /***
                     * If delete a photo or a template from the photos scene or the templates scene, we should update
                     * the album views and thumb views at the same time.
                     */
                    if (SceneType_Albums > m_type)
                    {
                        int n = 0;
                        ProxyWidgetsMap &albumProxyWidgets = m_scensVector.at(SceneType_Albums)->getProxyWidgets();

                        picFile = picLabel->getPictureFile();

                        foreach (PictureProxyWidget *albumProxyWidget, albumProxyWidgets)
                        {
                            albumProxyWidget->excludeItem(picFile);
                            if (albumProxyWidget->isEmpty())
                            {
                                n++;
                            }
                        }

                        qDebug() << __FILE__ << __LINE__ << n;

                        if (n == albumProxyWidgets.size()) // None photos and templates is existing in per album page
                        {
                            ProxyWidgetsMap &photoProxyWidgets = m_scensVector.at(SceneType_Photos)->getProxyWidgets();
                            foreach (PictureProxyWidget *photoProxyWidget, photoProxyWidgets)
                            {
                                photoProxyWidget->clearTimes();
                            }

                            ProxyWidgetsMap &tmplProxyWidgets = m_scensVector.at(SceneType_Templates)->getProxyWidgets();
                            foreach (PictureProxyWidget *tmplProxyWidget, tmplProxyWidgets)
                            {
                                tmplProxyWidget->clearTimes();
                            }
                        }

                        if (SceneType_Templates == m_type)
                        {
                            removeProxyWidget(getProxyWidget(m_proxyWidgets, picFile));

                            int pos = picFile.lastIndexOf(".png", -1, Qt::CaseInsensitive);
                            if (-1 != pos)
                            {
                                picFile.replace(pos, strlen(".png"), PKG_FMT);
                                m_filesList.removeOne(picFile);
                                //qDebug() << __FILE__ << __LINE__ << "remove" << picFile;
                            }

                            itemsList = all ? items() : selectedItems();

                            continue;
                        }
                        else
                        {
                            if (pEditPage && !picFile.isEmpty())
                            {
                                //qDebug() << __FILE__ << __LINE__ << "remove" << picFile;
                                pEditPage->removeThumbs(picFile);
                            }

                            m_filesList.removeOne(picFile);
                        }
                    }

                    if (SceneType_Albums <= m_type)
                    {
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

                        if (SceneType_Thumbs == m_type && !all/* Not is clear */)
                        {
                            photosList.append(picFile = picLabel->getPictureFile());
                            m_filesList.removeOne(picFile);
                        }

                        if (photosList.size() || tmplsList.size())
                        {
                            excludeItems(SceneType_Photos, photosList);
                            excludeItems(SceneType_Templates, tmplsList);
                        }

                        if (pEditPage && !picFile.isEmpty())
                        {
                            //qDebug() << __FILE__ << __LINE__ << "remove" << picFile;
                            pEditPage->removeThumbs(picFile);
                            itemsList = all ? items() : selectedItems();
                            continue;
                        }
                    }

                    m_proxyWidgets.remove(childWidget.getIndex());
                    childWidget.remove();
                }
            }

            removeItem(item);
            delete item;
            item = NULL;
            itemsList.removeFirst();
        }
    }

    l2m(m_proxyWidgets);
}

void PictureGraphicsScene::excludeItems(SceneType type, const QStringList &filesList)
{
    if (SceneType_Templates >= type)
    {
        foreach (const QString &file, filesList)
        {
            ProxyWidgetsMap &proxyWidgets = m_scensVector.at(type)->getProxyWidgets();
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

void PictureGraphicsScene::clearProxyWidgets()
{
    PictureProxyWidget *proxyWidget = NULL;
    GraphicsItemsList itemsList = items();

    foreach (QGraphicsItem *item, itemsList)
    {
        if ((proxyWidget = static_cast<PictureProxyWidget *>(item)))
        {
            proxyWidget->getChildWidget().updateBorder(false);
        }

        removeItem(item);
    }

    m_resultsWidgets.clear();
}

void PictureGraphicsScene::getChanges(QVariantList &changesList)
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
