#ifndef PICTUREPROXYWIDGET_H
#define PICTUREPROXYWIDGET_H

#include <QGraphicsProxyWidget>
#include <QTimeLine>
#include "events.h"

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

    static QGraphicsScene *getFocusScene(void){return m_focusScene;}

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
    void selectItem(void);
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
    static QGraphicsScene *m_focusScene;
};

#endif // PICTUREPROXYWIDGET_H
