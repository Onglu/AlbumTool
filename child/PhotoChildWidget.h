#ifndef PHOTOITEMWIDGET_H
#define PHOTOITEMWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"

namespace Ui {
    class PhotoChildWidget;
}

class PhotoChildWidget : public PictureChildWidget
{
    Q_OBJECT

public:
    explicit PhotoChildWidget(int index,
                              const QString &file,
                              qreal angle = 0,
                              Qt::Axis axis = Qt::ZAxis,
                              int usedTimes = 0,
                              TaskPageWidget *parent = 0);
    ~PhotoChildWidget();

    // 显示按钮
    void showButtons(bool bVisible);

    const QVariantMap &getChanges(void);

    // 获取当前使用次数
    int usedTimes(void) const;

    // 设置照片信息
    static void setPhotoInfo(DraggableLabel &label,
                             const QString &file,
                            qreal angle = 0,
                            Qt::Axis axis = Qt::ZAxis,
                            int usedTimes = 0);

protected slots:
    void onAccept(const QVariantMap &belongings);

private slots:
    void on_leftRotationPushButton_clicked();

    void on_rightRotationButton_clicked();

    void on_mirroredPushButton_clicked();

    void on_deletePushButton_clicked();

private:
    void rotate(qreal angle, Qt::Axis axis = Qt::ZAxis, bool report = true);

    Ui::PhotoChildWidget *ui;
};

class PhotoProxyWidget : public PictureProxyWidget
{
public:
    PhotoProxyWidget(PhotoChildWidget *photoWidget);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    PhotoChildWidget *m_pChildWidget;
};

#endif // PHOTOITEMWIDGET_H
