#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include <QLabel>

class PictureLabel : public QLabel
{
public:
    explicit PictureLabel(QWidget *parent = 0) : QLabel(parent), m_angle(0), m_opacity(1){}
    
    bool loadPicture(const QPixmap &pix, QSize size);

    bool hasPicture(void) const {return !m_ori.isNull();}

    bool hasPicture(const QString &fileName) const {return (m_fileName == fileName && !m_ori.isNull());}

    const QString &getPictureFile(void) const {return m_fileName;}

    const QPixmap &getPicture(bool bk = true){return bk ? m_bk : m_ori;}

    QSize getSize(void) const {return m_size;}

    void setOpacity(qreal opacity){m_opacity = opacity;}
    qreal getOpacity(void) const {return m_opacity;}

    void setAngle(qreal angle){m_angle = angle;}
    qreal getAngle(void) const {return m_angle;}

    /***
     * scale: not zoom(0 = scale), zoom in(0 < scale < 1), zoom fit(scale == 1), zoom out(1 < scale < 2)
     * all zooming operations which excepts 0 and 1 is based on its previous picture size.
     */
    void scaledZoom(float scale, Qt::AspectRatioMode aspectRatioMode);

    void setOpacity(QPixmap &pix, qreal opacity);

    void rotate(qreal angle, Qt::Axis axis = Qt::ZAxis);

    void clear(void)
    {
        m_angle = 0;
        m_opacity = 1;
        m_ori = m_bk = QPixmap();
        QLabel::clear();
    }

protected:
    QSize m_size;
    QString m_fileName;
    QPixmap m_ori, m_bk;

private:
    qreal m_angle, m_opacity;
};

#endif // PICTURELABEL_H
