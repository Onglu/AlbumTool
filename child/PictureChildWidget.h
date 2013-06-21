#ifndef PICTURECHILDWIDGET_H
#define PICTURECHILDWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QVariant>
#include "wrapper/DraggableLabel.h"

class TaskPageWidget;
class PictureChildWidget;
typedef QMap<int, PictureChildWidget *> ChildWidgetsMap;

class PictureChildWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PictureChildWidget(QSize fixedSize,
                                bool droppable = true,
                                TaskPageWidget *container = 0);

    TaskPageWidget *getOwner(void) const {return m_container;}

    void setIndex(int index);
    int getIndex(void) const{return m_index;}

    DraggableLabel *getPictureLabel(void) const {return m_picLabel;}

    virtual const QVariantMap &getChanges(void)
    {
        m_records.insert("index", m_index);
        return m_records;
    }

    void updateBorder(bool bSelected)
    {
        m_borderColor = bSelected ? Qt::darkCyan : Qt::transparent;
        update();
    }

    bool clickPicture(void)
    {
        emit itemSelected(Qt::ControlModifier == QApplication::keyboardModifiers() ? false/* Multi-selection */ : true);
        return true;
    }

    bool dblClickPicture(void)
    {
        emit itemDblSelected();
        return true;
    }

    virtual void open(ChildWidgetsMap &widgetsMap);
    virtual void swap(DraggableLabel &dragger);
    virtual void remove(void){}

    void unselectSelf(void){emit itemUnselected();}

signals:
    void itemSelected(bool bSingle);
    void itemDblSelected(void);
    void itemUnselected(void);
    void itemDetached(void);

protected slots:
    virtual void onAccept(int usedTimes);

protected:
    virtual bool meetDragDrop(QDropEvent *event);
    void mousePressEvent(QMouseEvent *){clickPicture();}
    void mouseDoubleClickEvent(QMouseEvent *){dblClickPicture();}
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);

    void setIndexLabel(int index, QLabel *lable, QPoint pos = QPoint(0, 0));
    void setPictureLabel(/*const QString &picFile*/ const QPixmap &pix,
                         QSize scaledSize,
                         const QString &mimeType,
                         QWidget *parent,
                         QPoint pos = QPoint(0, 0));

    int m_index;
    bool m_dragging, m_dropped;

    TaskPageWidget *m_container;
    QLabel *m_indexLabel;
    DraggableLabel *m_picLabel;
    QVariantMap m_records;

private:
    QBrush m_borderColor;
    QPalette m_defaultPalette;
};

#endif // PICTURECHILDWIDGET_H
