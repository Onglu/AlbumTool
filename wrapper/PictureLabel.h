#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include <QLabel>

#define MARGIN_SPACE    5

class PictureLabel : public QLabel
{
public:
    explicit PictureLabel(QWidget *parent = 0) : QLabel(parent), m_angle(0), m_axis(Qt::ZAxis), m_opacity(1){}
    
    bool loadPicture(const QPixmap &pix, QSize size);

    bool hasPicture(void) const {return !m_ori.isNull();}

    bool hasPicture(const QString &fileName) const {return (m_fileName == fileName && !m_ori.isNull());}

    const QString &getPictureFile(void) const {return m_fileName;}

    virtual const QPixmap &getPicture(bool bk = true){return bk ? m_bk : m_ori;}

    QSize getSize(void) const {return m_size;}

    void setOpacity(QPixmap &pix, qreal opacity) const;
    void setOpacity(qreal opacity){m_opacity = opacity;}
    float getOpacity(void) const {return m_opacity;}

    void rotate(qreal angle, Qt::Axis axis = Qt::ZAxis);
    void setAngle(qreal angle){m_angle = angle;}
    qreal getAngle(void) const {return m_angle;}

    /***
     * ratio: not zoom(0 = scale), zoom out(0 < scale < 1), zoom restore(scale == 1), zoom in(1 < scale < 2)
     * all zooming operations which excepts 0 and 1 is based on its previous picture size.
     */
    void scale(float ratio, Qt::AspectRatioMode aspectRatioMode);

protected:
    QSize m_size, m_default, m_scaled;
    QString m_fileName;
    QPixmap m_ori, m_bk;
    Qt::Axis m_axis;
    qreal m_angle;
    float m_opacity;

private:
//    qreal m_angle;
//    float m_opacity;
};

#endif // PICTURELABEL_H
