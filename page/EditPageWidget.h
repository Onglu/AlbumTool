#ifndef EDITPAGEWIDGET_H
#define EDITPAGEWIDGET_H

#include <QWidget>
#include "page/TemplatePageWidget.h"
#include "child/PhotoChildWidget.h"
#include "child/AlbumChildWidget.h"
#include "child/TemplateChildWidget.h"
#include "wrapper/BgdLayer.h"

class PictureGraphicsScene;
class TaskPageWidget;
class QPushButton;

namespace Ui {
    class EditPageWidget;
}

class EditPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditPageWidget(TaskPageWidget *container);
    ~EditPageWidget();

    int getViewWidth(void) const;

    PictureGraphicsScene *getThumbsView(void) const {return m_pThumbsScene;}

    void removeThumbs(const QString &picFile);

    /* Update the edit page views with the persent albums list */
    void updateViews(const ChildWidgetsMap &albumsMap, int current);

    /* Update the pictures list for the persent selected album */
    void updateAlbum(void);

signals:
    void editEntered(bool);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *);

private slots:
    void on_editPushButton_clicked();

    void on_photoPushButton_clicked();

    void on_templatePushButton_clicked();

    void on_backPushButton_clicked();

    void on_previousPushButton_clicked();

    void on_nextPushButton_clicked();

    void on_deletePushButton_clicked();

    void selectThumb(bool bSingle){m_thumbsSceneFocused = bSingle;}

    /* Replaced picture file */
    void onReplaced(const QString &current, const QString &replaced);

    void onRefreshed(PhotoLayer &label);

    void onClicked(PhotoLayer &label, QPoint pos);

    void on_mirroredPushButton_clicked();

    void on_resetPushButton_clicked();

    void on_zoomInPushButton_clicked();

    void on_zoomOutPushButton_clicked();

//    void on_zoomInPushButton_pressed();

//    void on_zoomInPushButton_released();

//    void on_zoomOutPushButton_pressed();

//    void on_zoomOutPushButton_released();

private:
    void updateLayers(void);

    void enableButtons(bool enable = true);

    void releaseButton(const QPushButton &button);

    bool swicth(int index);

    void adjustViewLayout(void);

    void adjustThumbsHeight(void);

    void showPhotos(bool visiable)
    {
        for (int i = 0; i < PHOTOS_NUMBER; i++)
        {
            if (visiable)
            {
                m_layerLabels[i]->show();
            }
            else
            {
                m_layerLabels[i]->hide();
            }
        }
    }

    Ui::EditPageWidget *ui;

    TaskPageWidget *m_container;
    AlbumChildWidget *m_pAlbumWidget;
    TemplatePageWidget *m_pTemplatePage;
    PictureGraphicsScene *m_pThumbsScene;

    bool m_thumbsSceneFocused;
    int m_current, m_x, m_y;
    QPoint m_startPos;
    ChildWidgetsMap m_albumsMap;

    QVariantList m_layers, m_photoLayers;
    LabelsVector m_layerLabels;
    PhotoLayer *m_layerLabel;
    BgdLayer *m_bgdLabel;

    friend class TaskPageWidget;
};

#endif // EDITPAGEWIDGET_H
