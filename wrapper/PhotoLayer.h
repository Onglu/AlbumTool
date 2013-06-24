#ifndef PHOTOLAYER_H
#define PHOTOLAYER_H

#include <QVariant>
#include "PictureLabel.h"

#define MAX_VISIABLE_RECT_TYPES      4

//class EditPageWidget;

class PhotoLayer : public PictureLabel
{
    Q_OBJECT

public:
    PhotoLayer(QWidget *parent) : PictureLabel(parent),
        m_moveable(false), m_borderColor(Qt::transparent)
    {
//        setFrameShape(QFrame::Box);
//        setFrameShadow(QFrame::Raised);
    }

    enum VisiableRectType{VisiableRectTypeDefault,
                          VisiableRectTypeContainer,
                          VisiableRectTypeCanvas,
                          VisiableRectTypeCopied,
                          VisiableRectTypeFixed,
                          VisiableRectTypeMax};

    const QImage &loadPhoto(const QVariantMap &layer,
                            QSizeF ratioSize,
                            QRect bgdRect,
                            const QString &replaced = QString());

    void changePhoto(const QVariantMap &belongings);

    QRect visiableRect(VisiableRectType type = VisiableRectTypeDefault) const
    {
        Q_ASSERT(VisiableRectTypeMax > type);
        return m_visiableRects[type];
    }

    const QImage &visiableImg(void) const {return m_visiableImg;}

//    qreal getOpacity(void) const {return m_opacity;}

//    qreal getAngle(void) const {return m_angle;}

    void flipAction(void){rotate(180.0f, Qt::YAxis);}

    void setMoveable(bool moveable);

    bool isMoveable(void) const {return m_moveable;}

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
    void blend(void);

    void updateVisiableRect(void);

    void updateCopiedRect(void);

    //EditPageWidget *m_container;
    QString m_replacedFile;

    bool m_moveable;
    QBrush m_borderColor;

    QSizeF m_ratioSize; // Visiable size devides actual size
    QRect m_bgdRect, m_maskRect, m_copiedRect, m_visiableRects[MAX_VISIABLE_RECT_TYPES], m_frameRect[2];
    qreal m_opacity, m_angle;
    QImage m_maskImg, m_composedImg, m_visiableImg;
};

class BgdLayer : public PictureLabel
{
public:
    BgdLayer(QWidget *parent) : PictureLabel(parent), m_enter(false), m_borderColor(Qt::transparent)
      , m_realSize(1, 1), m_ratioSize(1, 1)
    {
    }

    void setRealSize(QSizeF realSize){m_realSize = realSize;}
    QSizeF getRatioSize(void) const {return m_ratioSize;}

    /* fit rect */
    void setRect(QRect bgdRect){m_bgdRect = bgdRect;}
    QRect getRect(void) const {return m_bgdRect;}

    void setAngle(qreal angle){m_angle = angle;}
    void setOpacity(qreal opacity){m_opacity = opacity;}

    /* get the scaled size after loaded picture */
    /*QSize*/void loadPixmap(/*const QString &fileName*/ const QPixmap &pix)
    {
        if (loadPicture(/*fileName*/ pix, size()))
        {
            m_bgdImg = m_bk.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            m_ratioSize.setWidth((qreal)m_size.width() / m_realSize.width());
            m_ratioSize.setHeight((qreal)m_size.height() / m_realSize.height());
            //return m_size;
        }

        //return QSize(0, 0);
    }

    void enterCopiedRect(bool enter,
                         const QImage &visiableImg = QImage(),
                         QRect visiableRect = QRect())
    {
        m_enter = enter;

        if (enter)
        {
            m_visiableImg = visiableImg;
            m_visiableRect = visiableRect;
        }

        repaint();
    }

    void compose(const QVariantList &layers, const QVector<PhotoLayer *> labels);

    void flush(void)
    {
        m_visiableImg = m_composedImg = m_bgdImg = QImage();
        m_realSize = m_ratioSize = QSizeF(1, 1);
        m_angle = 0;
        m_opacity = 1;
        m_bgdRect.setRect(0, 0, 0, 0);
        clear();
    }

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool m_enter;
    QBrush m_borderColor;
    QImage m_visiableImg, m_bgdImg, m_composedImg;
    QRect m_visiableRect, m_bgdRect;
    QSizeF m_realSize, m_ratioSize; // background scale ratio size
};

#endif // PHOTOLAYER_H
