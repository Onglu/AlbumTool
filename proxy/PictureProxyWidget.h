#ifndef PICTUREPROXYWIDGET_H
#define PICTUREPROXYWIDGET_H

#include <QGraphicsProxyWidget>
#include <QTimeLine>

const QEvent::Type CustomEvent_Item_Selected = (QEvent::Type)(QEvent::User + 1);
const QEvent::Type CustomEvent_Item_Detached = (QEvent::Type)(QEvent::User + 2);
//const QEvent::Type CustomEvent_Load_Finished = (QEvent::Type)(QEvent::User + 3);

class PictureChildWidget;

class PictureProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
public:
    PictureProxyWidget(PictureChildWidget *child, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

    QRectF boundingRect(void) const
    {
        return QGraphicsProxyWidget::boundingRect().adjusted(-8, -8, 8, 8);
    }

    PictureChildWidget &getChildWidget(void) const {return *m_pChildWidget;}

    PictureChildWidget *getChildWidgetPtr(void) const {return m_pChildWidget;}

    static QGraphicsScene *getFocusScene(void){return m_pFocusScene;}

    static QTransform &getTransform(void){return m_trans;}

    bool isDetachable(void) const {return m_detachable;}

    virtual bool excludeItem(const QString &picFile);

    void clearTimes(void);

    bool isEmpty(void) const {return m_empty;}

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    bool m_empty;

protected slots:
    void selectItem(bool bSingle);
    void dblSelectItem(void);
    void unselectItem(void);
    void detachItem(void);

private slots:
    void updateStep(qreal step);
    void stateChanged(QTimeLine::State);
    void zoomIn(void);
    void zoomOut(void);

private:
    PictureChildWidget *m_pChildWidget;
    QTimeLine *m_pTimeLine;
    bool m_detachable;
    static QTransform m_trans;
    static QGraphicsScene *m_pFocusScene;
};

#endif // PICTUREPROXYWIDGET_H
