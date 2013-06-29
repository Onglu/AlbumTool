#ifndef DRAGGABLELABEL_H
#define DRAGGABLELABEL_H

#include <QLabel>
#include <QVariant>

#define DRAGGABLE_PHOTO         "image/x-draggable-photo-83207"
#define DRAGGABLE_TEMPLATE      "image/x-draggable-template-23807"
#define DRAGGABLE_ALBUM         "image/x-draggable-album-80732"
#define TEXT_SEP                '|'

class DraggableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit DraggableLabel(QWidget *parent = 0): QLabel(parent){}
    DraggableLabel(QSize size, const QString &mimeType, QWidget *parent = 0);
    DraggableLabel(const QPixmap &pix, QSize size, const QString &mimeType, QWidget *parent = 0);

    bool hasPicture(void) const;
    bool meetDragDrop(const QString &mimeType) const;

    void setPicture(const QPixmap &pix, QSize size = QSize(0, 0));
    QPixmap getPicture(bool scaled = true) const;

    QString getPictureFile(void) const
    {
        if (m_belongings.contains("picture_file"))
        {
            return m_belongings["picture_file"].toString();
        }

        return "";
    }

    QString getMimeType(void) const {return m_mimeType;}
    bool hasMimeType(void) const {return !m_mimeType.isEmpty();}
    void clearMimeType(void){if (hasMimeType()){m_mimeType.clear();}}

    void reset(void);

    /* Accept drag-drop require and handle it */
    void accept(bool inner = false);

    void setBelongings(const QVariantMap &belongings){m_belongings = belongings;}
    QVariantMap &getBelongings(void){return m_belongings;}

signals:
    void hasAccepted(const QVariantMap &belongings);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    bool meetDragDrop(void) const;

    QBrush m_bgdColor;
    QPixmap m_pix;  // original pixmap
    QString m_mimeType;
    QPoint m_startPos;
    QVariantMap m_belongings;
};

#endif // DRAGGABLELABEL_H
