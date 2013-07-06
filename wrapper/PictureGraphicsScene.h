#ifndef PICTUREGRAPHICSSCENE_H
#define PICTUREGRAPHICSSCENE_H

#include <QVector>
#include <QGraphicsScene>
#include "proxy/PictureProxyWidget.h"

class QGraphicsView;
class EditPageWidget;
class PictureGraphicsScene;
typedef QList<QGraphicsItem *> GraphicsItemsList;
typedef QMap<int, PictureProxyWidget *> ProxyWidgetsMap;
typedef QVector<PictureGraphicsScene *> GraphicsScenesVector;

class PictureGraphicsScene : public QGraphicsScene
{
public:
    enum LayoutMode{LayoutMode_Grid, LayoutMode_Horizontality};
    enum SceneType{SceneType_Photos,
                   SceneType_Templates,
                   SceneType_Albums,
                   SceneType_Thumbs,
                   SceneType_End};

    PictureGraphicsScene(const QBrush &bkclr,
                         LayoutMode layout,
                         SceneType type,
                         QGraphicsView *view = 0,
                         QObject *parent = 0);

    void updateScenes(GraphicsScenesVector &scenesVector){m_scensVector = scenesVector;}

    bool isEmpty(void) const {return (0 >= this->items().size());}

    void addProxyWidget(int index, PictureProxyWidget *proxyWidget);

    void insertProxyWidget(int index,
                           PictureProxyWidget *proxyWidget,
                           QString file = "");

    void removeProxyWidget(int index){removeProxyWidget(m_proxyWidgets[index]);}
    void removeProxyWidget(PictureProxyWidget *proxyWidget);
    void removeProxyWidgets(bool all, EditPageWidget *pEditPage = NULL);

    void clearFocusSelection(bool all);
    void clearProxyWidgets();

    void ok(bool loadFinished){m_loadFinished = loadFinished;}
    bool done(void) const {return m_loadFinished;}

    void autoAdjust(void)
    {
        if (!m_loadFinished)
        {
            adjustViewLayout();
        }
        else
        {
            adjustItemPos();
        }
    }

    void adjustItemPos(bool partial = false);
    void adjustViewLayout(int viewWidth = 0/* Indicates that the default width value will be used */);

    const GraphicsItemsList &m2l(const ProxyWidgetsMap &proxyWidgets);
    const ProxyWidgetsMap &l2m(ProxyWidgetsMap &proxyWidgets);

    ProxyWidgetsMap &getProxyWidgets(void){return m_proxyWidgets;}

    ProxyWidgetsMap &getResultWidgets(void){return m_resultsWidgets;}

    const QStringList &filesList(void){return m_filesList;}

    void getChanges(QVariantList &changesList);

private:
    int getViewWidth(void) const;

    void excludeItems(SceneType type, const QStringList &filesList);

    LayoutMode m_layout;
    const SceneType m_type;

    GraphicsScenesVector m_scensVector;
    ProxyWidgetsMap m_proxyWidgets, m_resultsWidgets;
    GraphicsItemsList m_itemsList;  // Just only used for m2l() convertion
    QStringList m_filesList;
    bool m_loadFinished;
    QGraphicsView *m_pView;
};

#endif // PICTUREGRAPHICSSCENE_H
