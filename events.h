#ifndef EVENTS_H
#define EVENTS_H

#include <QEvent>

const QEvent::Type CustomEvent_Item_Selected = (QEvent::Type)(QEvent::User + 1);
const QEvent::Type CustomEvent_Item_Detached = (QEvent::Type)(QEvent::User + 2);
const QEvent::Type CustomEvent_Item_Replaced = (QEvent::Type)(QEvent::User + 3);
const QEvent::Type CustomEvent_Item_Unknown = (QEvent::Type)(QEvent::User + 10);
const QEvent::Type CustomEvent_Load_BEGIN = (QEvent::Type)(QEvent::User + 11);
const QEvent::Type CustomEvent_MAKE_BEGIN = (QEvent::Type)(QEvent::User + 12);
//const QEvent::Type CustomEvent_Load_Finished = (QEvent::Type)(QEvent::User + 3);

class ReplacerEvent : public QEvent
{
public:
    ReplacerEvent(Type type, const QString &current, const QString &replaced) :
        QEvent(type),
        m_current(current),
        m_replaced(replaced)
    {
    }

    const QString &getCurrent(){return m_current;}

    const QString &getReplaced(){return m_replaced;}

private:
    QString m_current, m_replaced;
};

#endif // EVENTS_H
