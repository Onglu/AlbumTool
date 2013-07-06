#ifndef THUMBCHILDWIDGET_H
#define THUMBCHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"

typedef PictureProxyWidget ThumbProxyWidget;

class ThumbChildWidget : public PictureChildWidget
{
    Q_OBJECT

public:
    explicit ThumbChildWidget(int index,
                              const QString &mimeType,
                              const QString &file,
                              qreal angle = 0,
                              Qt::Axis axis = Qt::ZAxis,
                              TaskPageWidget *parent = 0);

    const QVariantMap &getBelongings(void){return m_belongings;}

    void setId(int id){m_id = id;/* zero-based index */}
    int getId(void) const {return m_id;}

    static const QString &getReplaced(void){return m_replaced;}

    static void getRotation(qreal &angle, Qt::Axis &axis)
    {
        angle = m_angle;
        axis = m_axis;
    }

    static void updateList(const QStringList &photosList){m_photosList = photosList;}

signals:
    void itemReplaced(const QString &current, const QString &replaced);

protected:
    void dropEvent(QDropEvent *event);

private:
    int m_id;   // A zero-based id number of every photo which added in a album photoslist
    QVariantMap m_belongings;
    static QStringList m_photosList;
    static QString m_replaced;
    static qreal m_angle;
    static Qt::Axis m_axis;
};

#endif // THUMBCHILDWIDGET_H
