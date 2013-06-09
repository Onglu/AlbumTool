#ifndef EDITPAGEWIDGET_H
#define EDITPAGEWIDGET_H

#include <QWidget>
#include "page/TemplatePageWidget.h"
#include "child/PhotoChildWidget.h"
#include "child/AlbumChildWidget.h"
#include "child/TemplateChildWidget.h"

class PictureGraphicsScene;
class TaskPageWidget;
class PhotoLayer;
class BgdLayer;
typedef QVector<PhotoLayer *> LayerLabelsVector;

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
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

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

    void labelClicked(PhotoLayer &label, QPoint pos);

    void on_mirroredPushButton_clicked();

private:
    void updateLayers(void);

    bool swicth(int index);

    void adjustViewLayout(void);

    void adjustThumbsHeight(void);

    Ui::EditPageWidget *ui;

    TaskPageWidget *m_container;
    AlbumChildWidget *m_pAlbumWidget;
    TemplatePageWidget *m_pTemplatePage;
    PictureGraphicsScene *m_pThumbsScene;

    bool m_thumbsSceneFocused;
    int m_current, m_x, m_y;
    QPoint m_startPos;
    ChildWidgetsMap m_albumsMap;
    QString m_tmplPic, m_bgdPic;    /* Picture file name */

    QVariantList m_layers, m_photoLayers;
    LayerLabelsVector m_layerLabels;
    PhotoLayer *m_layerLabel;
    BgdLayer *m_bgdLabel;

    friend class TaskPageWidget;
};

#endif // EDITPAGEWIDGET_H
