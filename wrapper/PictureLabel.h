#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include <QLabel>

class PictureLabel : public QLabel
{
public:
    explicit PictureLabel(QWidget *parent = 0);
    
    bool loadPicture(/*const QString &fileName*/ const QPixmap &pix, QSize size);

    bool hasPicture(void) const {return !m_ori.isNull();}

    bool hasPicture(const QString &fileName) const {return (m_fileName == fileName && !m_ori.isNull());}

    const QString &getPictureFile(void) const {return m_fileName;}

    const QPixmap &getPicture(bool bk = true){return bk ? m_bk : m_ori;}

    QSize getSize(void) const {return m_size;}

protected:
    /***
     * scale: not zoom(0 = scale), zoom in(0 < scale < 1), zoom fit(scale == 1), zoom out(1 < scale < 2)
     * all zooming operations which excepts 0 and 1 is based on its previous picture size.
     */
    void scaledZoom(float scale);

    void setOpacity(QPixmap &pix, qreal opacity);

    void rotate(qreal angle, Qt::Axis axis = Qt::ZAxis);

    QSize m_size, m_optimal;
    QString m_fileName;
    QPixmap m_ori, m_bk;
    qreal m_angle, m_opacity;

private:

};

#endif // PICTURELABEL_H
