#ifndef BGDLAYER_H
#define BGDLAYER_H

#include "PhotoLayer.h"

typedef QVector<PhotoLayer *> LabelsVector;

class BgdLayer : public PictureLabel
{
public:
    BgdLayer(PhotoLayer::VisiableImgType type, QWidget *parent) : PictureLabel(parent),
        m_enter(false),
        m_borderColor(Qt::transparent),
        m_type(type),
        m_actualSize(1, 1),
        m_ratioSize(1, 1),
        m_locations(0)
    {
        setFrameShape(QFrame::Box);
        setFrameShadow(QFrame::Raised);
    }

    PhotoLayer::VisiableImgType getVisiableImgType(void) const {return m_type;}

    void setActualSize(QSizeF actualSize)
    {
        if (PhotoLayer::VisiableImgTypeOriginal == m_type)
        {
            setFixedSize(actualSize.toSize());
        }

        m_actualSize = actualSize;
    }

    QSizeF getRatioSize(void) const {return m_ratioSize;}

    /* get the scaled size after loaded picture */
    void loadPixmap(const QPixmap &pix);

    void loadLayers(int locations/* The number of used photos */,
                    const QVariantList &layers,
                    const LabelsVector &labels)
    {
        m_locations = locations;
        m_layers = layers;
        m_labels = labels;
    }

    void enterCopiedRect(bool enter, QRect getVisiableRect = QRect())
    {
        m_enter = enter;

        if (enter)
        {
            m_visiableRect = getVisiableRect;
        }

        repaint();
    }

    void compose(const QString &fileName = QString());

    void flush(void)
    {
        m_srcImg/*[PhotoLayer::VisiableImgTypeOriginal] = m_srcImg[PhotoLayer::VisiableImgTypeScreen]*/ = QImage();
        m_actualSize = m_ratioSize = QSizeF(1, 1);
        clear();
    }

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool m_enter;
    QBrush m_borderColor;
    const PhotoLayer::VisiableImgType m_type;
    QImage m_srcImg/*[VISIABLE_IMG_TYPES]*/;
    QRect m_visiableRect;
    QSizeF m_actualSize, m_ratioSize; // background scale ratio size
    int m_locations;
    QVariantList m_layers;
    LabelsVector m_labels;
};

#endif // BGDLAYER_H
