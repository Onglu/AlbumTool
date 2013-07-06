#ifndef EDITPAGEWIDGET_H
#define EDITPAGEWIDGET_H

#include <QWidget>
#include "TemplatePageWidget.h"
#include "child/AlbumChildWidget.h"
//#include "defines.h"
#include "wrapper/utility.h"

class PictureGraphicsScene;
class AlbumPageWidget;
class TaskPageWidget;
class PhotoLayer;
class QPushButton;

namespace EditPage
{
    class MakerThread : public QThread
    {
        Q_OBJECT

    public:
        MakerThread(void){}

        void replace(const QString &current, const QString &replaced)
        {
            m_current = current;
            m_replaced = replaced;
            this->start();
        }

    signals:
        void doing(const QString &, const QString &);

    protected:
        void run()
        {
            emit doing(m_current, m_replaced);
            emit finished();
        }

    private:
        QString m_current, m_replaced;
    };
}

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

    void updatePage(void);

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

    void onReplacing(const QString &current, const QString &replaced);

    /* Replaced picture file */
    void onReplaced(const QString &current, const QString &replaced);

    void onClicked(PhotoLayer &label, QPoint pos);

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

private:
    void enableButtons(bool enable = true);

    void releaseButton(const QPushButton &button);

    void zoomAction(QPushButton &button, bool in);

    bool switchPage(int index);

    void adjustViewLayout(void);

    void adjustThumbsHeight(void);

    Ui::EditPageWidget *ui;

    TaskPageWidget *m_container;
    PhotoLayer *m_layerLabel;
    AlbumChildWidget *m_pAlbumWidget;
    AlbumPageWidget *m_pAlbumPage;
    TemplatePageWidget *m_templatePage;
    PictureGraphicsScene *m_pThumbsScene;

    bool m_thumbsSceneFocused;
    int m_current, m_x, m_y;
    QPoint m_startPos;
    ChildWidgetsMap m_albumsMap;

    EditPage::MakerThread m_maker;

    friend class TaskPageWidget;
};

#endif // EDITPAGEWIDGET_H
