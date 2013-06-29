#ifndef BGDLAYER_H
#define BGDLAYER_H

#include "PhotoLayer.h"

typedef QVector<PhotoLayer *> LabelsVector;

class BgdLayer : public PictureLabel
{
public:
    BgdLayer(QWidget *parent) : PictureLabel(parent), m_enter(false), m_borderColor(Qt::transparent)
      , m_actualSize(1, 1), m_ratioSize(1, 1), m_locations(0)
    {
    }

    void setActualSize(QSizeF actualSize){m_actualSize = actualSize;}
    QSizeF getRatioSize(void) const {return m_ratioSize;}

    /* fit rect */
    void setRect(QRect bgdRect){m_bgdRect = bgdRect;}
    QRect getRect(void) const {return m_bgdRect;}

    /* get the scaled size after loaded picture */
    void loadPixmap(const QPixmap &pix)
    {
        if (loadPicture(pix, size()))
        {
            m_srcImg[VISIABLE_IMG_ORIGINAL] = m_ori.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            m_srcImg[VISIABLE_IMG_SCREEN] = m_bk.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            m_ratioSize.setWidth((qreal)m_size.width() / m_actualSize.width());
            m_ratioSize.setHeight((qreal)m_size.height() / m_actualSize.height());
        }
    }

    void loadLayers(int locations/* The number of used photos */,
                    const QVariantList &layers,
                    const LabelsVector &labels)
    {
        m_locations = locations;
        m_layers = layers;
        m_labels = labels;
    }

    void enterCopiedRect(bool enter, QRect visiableRect = QRect())
    {
        m_enter = enter;

        if (enter)
        {
            m_visiableRect = visiableRect;
        }

        repaint();
    }

    void compose(int type, const QString &fileName = QString());

    void flush(void)
    {
        m_srcImg[VISIABLE_IMG_ORIGINAL] = m_srcImg[VISIABLE_IMG_SCREEN] = QImage();
        m_actualSize = m_ratioSize = QSizeF(1, 1);
        clear();
    }

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool m_enter;
    QBrush m_borderColor;
    QImage m_srcImg[VISIABLE_IMG_TYPES];
    QRect m_visiableRect, m_bgdRect;
    QSizeF m_actualSize, m_ratioSize; // background scale ratio size
    int m_locations;
    QVariantList m_layers;
    LabelsVector m_labels;
};

#endif // BGDLAYER_H
