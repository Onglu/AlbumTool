#ifndef PHOTOLAYER_H
#define PHOTOLAYER_H

#include <QVariant>
#include "PictureLabel.h"

#define VISIABLE_IMG_SCREEN          0
#define VISIABLE_IMG_ORIGINAL        1
#define VISIABLE_IMG_TYPES           2
#define VISIABLE_RECT_TYPES          4

class PhotoLayer : public PictureLabel
{
    Q_OBJECT

public:
    PhotoLayer(QWidget *parent) : PictureLabel(parent),
        m_moveable(false), m_moved(false), m_borderColor(Qt::transparent), m_ratioSize(QSizeF(1, 1))
    {
//        setFrameShape(QFrame::Box);
//        setFrameShadow(QFrame::Raised);
    }

    enum VisiableRectType{VisiableRectTypeDefault,
                          VisiableRectTypeCanvas,
                          VisiableRectTypeCopied,
                          VisiableRectTypeFixed};

    bool loadPhoto(const QVariantMap &photoLayer,
                   QSizeF ratioSize,
                   QRect bgdRect,
                   const QString &replaced = QString());

    void changePhoto(const QVariantMap &belongings);

    bool hasPhoto(int type = VISIABLE_IMG_SCREEN){return !m_visiableImgs[type].isNull();}

    QRect visiableRect(VisiableRectType type = VisiableRectTypeDefault) const
    {
        return m_visiableRects[type];
    }

    const QImage &visiableImg(int type = VISIABLE_IMG_SCREEN) const {return m_visiableImgs[type];}

    QRect getFrame(void) const
    {
        return actualRect(m_visiableRects[VisiableRectTypeCanvas].center(),
                         m_visiableRects[VisiableRectTypeCanvas].size());
    }

    QRect actualRect(QRect rect) const
    {
        return actualRect(rect.topLeft(), rect.size());
    }

    QRect actualRect(QPointF pos, QSizeF size) const
    {
        pos.rx() /= m_ratioSize.width();
        pos.ry() /= m_ratioSize.height();
        size.rwidth() /= m_ratioSize.width();
        size.rheight() /= m_ratioSize.height();
        return QRectF(pos, size).toRect();
    }

    qreal getOpacity(void) const {return m_opacity;}

    qreal getAngle(void) const {return m_angle;}

    void flipAction(void){rotate(180.0f, Qt::YAxis);}

    void setMoveable(bool moveable);

    bool isMoveable(void) const {return m_moveable;}

    bool hasMoved(void){return m_moved;}

    void movePhoto(QPoint offset);

    void updateRect(void)
    {
        clear();

        if (m_size.width() > m_bgdRect.width() || m_size.height() > m_bgdRect.height())
        {
            updateVisiableRect();
        }
        else
        {
            updateCopiedRect();
        }
    }

signals:
    void hasUpdated(void);

    void clicked(PhotoLayer &self, QPoint pos);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void blend(bool actual = false);

    void updateVisiableRect(QRect copiedRect = QRect());

    void updateCopiedRect(void);

    QString m_replacedFile;

    bool m_moveable, m_moved;
    QBrush m_borderColor;

    QSizeF m_ratioSize; // Visiable size devides actual size
    QRect m_bgdRect, m_maskRect, m_copiedRect, m_visiableRects[VISIABLE_RECT_TYPES], m_frameRect[2];
    qreal m_opacity, m_angle;
    QImage m_maskImgs[VISIABLE_IMG_TYPES], m_visiableImgs[VISIABLE_IMG_TYPES], m_composedImg;
};

#endif // PHOTOLAYER_H
