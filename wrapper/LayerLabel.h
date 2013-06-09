#ifndef PHOTOLAYER_H
#define PHOTOLAYER_H

#include <QVariant>
#include "PictureLabel.h"

#define VISIABLE_RECT_TYPE_CANVAS    0
#define VISIABLE_RECT_TYPE_CONTAINER 1

//class EditPageWidget;

class LayerLabel : public PictureLabel
{
    Q_OBJECT

public:
    LayerLabel(QWidget *parent) : PictureLabel(parent), m_moveable(false), m_borderColor(Qt::transparent)
    {
//        setFrameShape(QFrame::Box);
//        setFrameShadow(QFrame::Raised);
//        m_flags = windowFlags();
    }

    const QImage &loadPicture(const QVariantMap &layer,
                              QSizeF ratioSize,
                              QRect bgdRect,
                              const QString &replaced = QString());

    void changePicture(const QVariantMap &belongings);

    void updateView(void);

    QRect visiableRect(uchar type) const
    {
        Q_ASSERT(VISIABLE_RECT_TYPE_CONTAINER >= type);
        return m_visiableRect[type];
    }

    const QImage &visiableImg(void) const {return m_visiableImg;}

//    qreal getOpacity(void) const {return m_opacity;}

//    qreal getAngle(void) const {return m_angle;}

    void flipAction(void){rotate(180.0f, Qt::YAxis);}

    void setMoveable(bool moveable);

    bool isMoveable(void) const {return m_moveable;}

signals:
    void hasUpdated(void);

    void clicked(LayerLabel &self, QPoint pos);

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void blendImg(void);

    //EditPageWidget *m_container;
    QString m_replacedFile;

    Qt::WindowFlags m_flags;

    bool m_moveable;
    QBrush m_borderColor;

    QSizeF m_ratioSize; // Visiable size devides actual size
    QRect m_bgdRect, m_maskRect, m_copiedRect, m_visiableRect[2], m_frameRect[2];
    qreal m_opacity, m_angle;
    QImage m_maskImg, m_composedImg, m_visiableImg;
};

#endif // PHOTOLAYER_H
