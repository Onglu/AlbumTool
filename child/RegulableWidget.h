#ifndef REGULABLEWIDGET_H
#define REGULABLEWIDGET_H

#include <QWidget>
#include <QVariant>

#define TOP_FRAME_HEIGHT             31
#define VISIABLE_RECT_TYPE_CANVAS    0
#define VISIABLE_RECT_TYPE_CONTAINER 1

class QLabel;
class EditPageWidget;
typedef QList<QPixmap> PhotosList;

namespace Ui {
class RegulableWidget;
}

class RegulableWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit RegulableWidget(EditPageWidget *m_container = 0);
    ~RegulableWidget();

    void showButtons(bool visiable = true);

    const QImage &loadPicture(const QVariantMap &layer,
                              QSizeF ratioSize,
                              QRect bgdRect,
                              const QString &replaced = QString());

    void changePicture(const QVariantMap &belongings);

    bool hasLoaded(void) const {return !m_ori.isNull();}

    bool hasLoaded(const QString &fileName) const {return (m_picFile == fileName && !m_ori.isNull());}

    void updateView(void);

    QRect visiableRect(uchar type) const
    {
        Q_ASSERT(VISIABLE_RECT_TYPE_CONTAINER >= type);
        return m_visiableRect[type];
    }

    const QString &photoFile(void){return m_picFile;}

    const QImage &visiableImg(void) const {return m_visiableImg;}

    void getShape(qreal &opacity, qreal &angle) const {opacity = m_opacity; angle = m_angle;}

    static void rotate(QLabel *label,
                       QPixmap &pix,
                       qreal angle,
                       Qt::Axis axis = Qt::ZAxis);

signals:
    void hasUpdated(void);

protected:
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter(QObject *, QEvent *);

private slots:
    void on_zoomInPushButton_clicked(){scaledZoom(1.25);}

    void on_zoomOutPushButton_clicked(){scaledZoom(0.75);}

    void on_scalelessPushButton_clicked(){scaledZoom(0);}

    void on_fitPushButton_clicked(){scaledZoom(1);}

    void on_mirroredPushButton_clicked();

    void on_movePushButton_clicked();

private:
    /***
     * scale: not zoom(0 = scale), zoom in(0 < scale < 1), zoom fit(scale == 1), zoom out(1 < scale < 2)
     * all zooming operations which excepts 0 and 1 is based on its previous picture size.
     */
    void scaledZoom(float scale);

    void blend(void);

    Ui::RegulableWidget *ui;
    EditPageWidget *m_container;

    int m_x, m_y;
    bool m_moveable;
    QSizeF m_ratioSize; // Visiable size devides actual size
    QRect m_bgdRect, m_maskRect, m_visiableRect[2];
    qreal m_opacity, m_angle;

    QString m_picFile;
    QPixmap m_bk, m_ori;
    QImage m_maskImg, m_composedImg, m_visiableImg;
};

#endif // REGULABLEWIDGET_H
