#ifndef EDITPAGEWIDGET_H
#define EDITPAGEWIDGET_H

#include <QWidget>
#include <QTimer>
#include "TemplatePageWidget.h"
#include "child/AlbumChildWidget.h"
#include "wrapper/utility.h"

class PictureGraphicsScene;
class AlbumPageWidget;
class TaskPageWidget;
class PhotoLayer;
class QPushButton;
class ThumbChildWidget;

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

    void exec(bool open = true);

signals:
    void editEntered(bool);

protected:
    void closeEvent(QCloseEvent *)
    {
        emit editEntered(false);
    }

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

    void selectThumb(void){m_thumbsSceneFocused = true;}

    void clickPicture(QPoint wpos, QPoint epos);

    /* Replaced picture file */
    void onReplaced(const QString &current, const QString &replaced);

    void on_mirroredPushButton_clicked();

    void on_resetPushButton_clicked();
#if 1
    void on_zoomInPushButton_clicked();

    void on_zoomOutPushButton_clicked();
#else
    void on_zoomInPushButton_pressed();

    void on_zoomInPushButton_released();

    void on_zoomOutPushButton_pressed();

    void on_zoomOutPushButton_released();
#endif

    void end();

    void on_angleLineEdit_returnPressed();

private:
    void enableButtons(bool enable = true);

    void releaseButton(const QPushButton &button);

    void zoomAction(QPushButton &button, bool in);

    void switchPage(int index);

    void updatePage(void);

    void adjustViewLayout(void);

    void adjustThumbsHeight(void);

    const ThumbChildWidget *getThumbWidget(const QString &picFile) const;

    Ui::EditPageWidget *ui;

    QWidget *m_loading;
    QTimer m_processor;

    TaskPageWidget *m_container;
    PhotoLayer *m_layerLabel;
    AlbumChildWidget *m_pAlbumWidget;
    AlbumPageWidget *m_pAlbumPage;
    TemplatePageWidget *m_templatePage;
    PictureGraphicsScene *m_pThumbsScene;

    bool m_thumbsSceneFocused;
    int m_current;
    QPoint m_startPos;
    ChildWidgetsMap m_albumsMap;

    friend class TaskPageWidget;
};

#endif // EDITPAGEWIDGET_H
