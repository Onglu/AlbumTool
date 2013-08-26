#ifndef PHOTOLAYER_H
#define PHOTOLAYER_H

#include <QVariant>
#include "PictureLabel.h"

#define VISIABLE_IMG_TYPES           2
#define VISIABLE_RECT_TYPES          4

class PhotoLayer : public PictureLabel
{
    Q_OBJECT

public:
    enum VisiableImgType{VisiableImgTypeScreen,
                         VisiableImgTypeOriginal};

    enum VisiableRectType{VisiableRectTypeDefault,
                          VisiableRectTypeCanvas,
                          VisiableRectTypeCopied,
                          VisiableRectTypeFixed};

    PhotoLayer(VisiableImgType type, QWidget *parent);

    void setCanvas(QSizeF ratioSize, QRect bgdRect)
    {
        if (!ratioSize.width())
        {
            ratioSize.setWidth(1);
        }

        if (!ratioSize.height())
        {
            ratioSize.setHeight(1);
        }

        m_ratioSize = ratioSize;
        m_bgdRect = bgdRect;
    }

    bool loadPhoto(bool restore,
                   const QVariantMap &photoLayer,
                   const QString &photoFile = QString(),
                   qreal angle = 0,
                   Qt::Axis axis = Qt::ZAxis,
                   QRect location = QRect()
                   );

    void changePhoto(const QString &photoFile,
                     qreal angle = 0,
                     Qt::Axis axis = Qt::ZAxis);

    bool hasPhoto(void) const {return !m_visiableImg.isNull();}

    const QString &getPhotoFile(void) const {return m_picFile;}

    const QPixmap &getPicture(bool bk)
    {
        if (m_angle)
        {
            return QPixmap(m_picFile);
        }

        return bk ? m_bk : m_ori;
    }

    QString getLayerId(void) const;

    QRect getVisiableRect(VisiableRectType type = VisiableRectTypeDefault) const
    {
        return m_visiableRects[type];
    }

    const QImage &getVisiableImg() const {return m_visiableImg;}

    QRect getFrame(void) const
    {
        return getActualRect(m_visiableRects[VisiableRectTypeCanvas].center(),
                             m_visiableRects[VisiableRectTypeCanvas].size());
    }

    QRect getActualRect(QRect rect) const
    {
        return getActualRect(rect.topLeft(), rect.size());
    }

    QRect getActualRect(QPointF pos, QSizeF size) const
    {
        pos.rx() /= m_ratioSize.width();
        pos.ry() /= m_ratioSize.height();
        size.rwidth() /= m_ratioSize.width();
        size.rheight() /= m_ratioSize.height();
        return QRectF(pos, size).toRect();
    }

    void setMoveable(bool moveable);

    bool isMoveable(void) const {return m_moveable;}

    bool hasMoved(void){return m_moved;}

    void movePhoto(QPoint offset);

    bool zoomAction(float ratio);

    void rotateAction(qreal angle, Qt::Axis axis = Qt::ZAxis);

    void updateRect(void);

    void flush(bool all = true);

    static PhotoLayer *getReplaced(PhotoLayer *&layer)
    {
        layer = m_replaced;
        m_replaced = NULL;
        return layer;
    }

signals:
    void clicked(QPoint wpos, QPoint epos);

    void replaced(const QString &tmplPic, const QString &tmplFile);

protected:
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void blend(void);

    void updateCanvasRect(bool restore = false);

    void updateVisiableRect(void);

    void updateCopiedRect(void);

    const VisiableImgType m_type;
    bool m_moveable, m_moved;
    QSizeF m_ratioSize; // Visiable size devides actual size
    QRect m_bgdRect, m_maskRect, m_visiableRects[VISIABLE_RECT_TYPES];
    QPixmap m_src;
    QImage m_visiableImg, m_maskImg, m_composedImg;
    QString m_picFile;

    static PhotoLayer *m_replaced;
};

#endif // PHOTOLAYER_H
