#ifndef EVENTS_H
#define EVENTS_H

const QEvent::Type CustomEvent_Item_Selected = (QEvent::Type)(QEvent::User + 1);
const QEvent::Type CustomEvent_Item_Detached = (QEvent::Type)(QEvent::User + 2);
const QEvent::Type CustomEvent_Item_Unknown = (QEvent::Type)(QEvent::User + 10);
const QEvent::Type CustomEvent_Load_BEGIN = (QEvent::Type)(QEvent::User + 11);
//const QEvent::Type CustomEvent_Load_Finished = (QEvent::Type)(QEvent::User + 3);

#endif // EVENTS_H
