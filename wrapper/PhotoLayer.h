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

    bool loadPhoto(const QVariantMap &photoLayer,
                   const QString &photoFile = QString(),
                   qreal angle = 0,
                   Qt::Axis axis = Qt::ZAxis,
                   int id = -1);

    void changePhoto(const QString &photoFile,
                     qreal angle = 0,
                     Qt::Axis axis = Qt::ZAxis);

    bool hasPhoto(void) const {return !m_visiableImg.isNull();}

    const QString &getPhotoFile(void) const {return m_picFile;}

    void setId(int id){m_id = id;}
    int getId(void) const {return m_id;}

    float getOpacity(void) const {return m_opacity;}

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

    bool zoomAction(float scale);

    void flipAction(void){rotate(180.0f, Qt::YAxis);}

    void updateRect(void);

    void flush(bool all = true);

    void blend(void);

    static PhotoLayer *getReplaced(PhotoLayer *&layer)
    {
        layer = m_replaced;
        m_replaced = NULL;
        return layer;
    }

signals:
    void clicked(PhotoLayer &self, QPoint pos);

    void replaced(const QString &tmplPic, const QString &tmplFile);

protected:
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void updateCanvasRect(void);

    void updateVisiableRect(QRect copiedRect = QRect());

    void updateCopiedRect(void);

    const VisiableImgType m_type;
    bool m_moveable, m_moved;
    float m_opacity;

    QSize m_default;
    QSizeF m_ratioSize; // Visiable size devides actual size
    QRect m_bgdRect, m_maskRect, m_visiableRects[VISIABLE_RECT_TYPES];
    QImage m_visiableImg, m_maskImg, m_composedImg;
    QString m_picFile;
    int m_id;

    static PhotoLayer *m_replaced;
};

#endif // PHOTOLAYER_H
