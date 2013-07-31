#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include <QImageReader>

class QGraphicsScene;
typedef QMap<QString, QPixmap> PicturesMap;
typedef QList<QPixmap> PicturesList;

namespace Ui {
class PreviewDialog;
}

class PreviewDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PreviewDialog(QWidget *parent = 0);
    PreviewDialog(const QStringList &files, int current, QWidget *parent = 0);
    ~PreviewDialog();

    void updateList(const QStringList &files, int current);

    void updateList(const PicturesList &pictures);

    int getCurrent(void) const {return m_current;}
    
protected:
    void timerEvent(QTimerEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *);
    void resizeEvent(QResizeEvent *);

private slots:
    void on_zoomInPushButton_clicked();

    void on_zoomOutPushButton_clicked();

    void on_zoomResetPushButton_clicked();

    void on_fitWindowPushButton_clicked();

    void on_fullScreenPushButton_clicked();

    void on_prevPushButton_clicked();

    void on_nextPushButton_clicked();

    void on_leftRotationPushButton_clicked();

    void on_rightRotationPushButton_clicked();

    void on_mirroredPushButton_clicked();

    void on_deletePushButton_clicked();

signals:
    void itemDetached(QGraphicsScene *scene, const QString &name);

private:
    void switchPage(int index);

    void switchPage(const QPixmap &picture);

    void rotate(qreal angle, Qt::Axis axis = Qt::ZAxis);

    Ui::PreviewDialog *ui;
    int m_nTimerId;

    QPixmap m_bk, m_ori;
    QImageReader m_reader;
    //QByteArray bytes;

    QRect m_rect;
    bool m_bFit, m_bDraggable;
    int m_x, m_y;
    Qt::TransformationMode m_transformMode;

    int m_current;
    QStringList m_filesList;
    PicturesList m_picturesList;
    QGraphicsScene *m_scene;
};

#endif // PREVIEWDIALOG_H
