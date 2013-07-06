#ifndef ALBUMITEMWIDGET_H
#define ALBUMITEMWIDGET_H

#include <QWidget>
#include <QSettings>
#include "parser/FileParser.h"
#include "page/EditPageWidget.h"
//#include "wrapper/utility.h"
#include "wrapper/PictureGraphicsScene.h"
#include <QDialog>

class LoadingDlg;
class LoadingDlg1;
class PreviewDialog;
class TemplateChildWidget;
class TaskPageWidget;

namespace TaskPage
{
    class MakerThread : public QThread
    {
        Q_OBJECT

    public:
        MakerThread(TaskPageWidget *page = 0) : m_page(page), m_abort(false){}

        void stop(){m_abort = true;}

    signals:
        void doing(void);

    protected:
        void run();
//        {
//            emit doing();
//            emit finished();
//        }

    private:
        TaskPageWidget *m_page;
        bool m_abort;
    };
}

namespace Ui {
class TaskPageWidget;
}

class TaskPageWidget : public QWidget
{
    Q_OBJECT

public:
    TaskPageWidget(int tabId, const QString &taskFile, QWidget *parent = 0);
    ~TaskPageWidget();

    bool isValid(void){return m_taskParser.isValid();}
    bool hasOpened(const QString &taskFile) const {return (taskFile == m_taskParser.getParsingFile());}
    bool isEditing(void) const {return m_editPage->isVisible();}

    void setTabId(int tabId){m_tabId = tabId;}
    void importTemplates(void);

    void onPreview(const QStringList &pictures, int current);
    void onEdit(const ChildWidgetsMap &albumsMap, int current);
    void onSearch(bool immediate, bool inner, const QVariantMap &tags);

    bool hasChanged(void) const {return m_changed;}
    char *saveFile(uchar mode);
    void saveChanges(void);
    void noticeChanged(void);

    bool replace(PictureGraphicsScene::SceneType type,
                 const QString &current,
                 const QString &replaced);

    TemplateChildWidget *getTmplWidget(const QString &tmplFile) const;

    const QString &getPageId(void) const {return m_taskParser.getPageId();}

    void showProcess(bool show,
                            const QString &info = QString());

    static bool deleteDir(const QString &dir, bool all = true);

    //void process();

signals:
    void changed(int index);
    void maxShow(bool max);

protected:
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void customEvent(QEvent *);

private slots:
    void on_collapsePushButton_clicked();

    void on_importPhotosPushButton_clicked();

    void on_addAlbumPushButton_clicked();

    void on_createPushButton_clicked();

    void on_previewPushButton_clicked();

    void detachItem(QGraphicsScene *scene, const QString &file);

    void updateViews(void);

    void enterEdit(bool enter);

    void addItem(int index, const QString &file, qreal angle, Qt::Axis axis, int usedTimes);

    void addItem(int index, const QString &file, int usedTimes);

    void addItem(int index,
                 const QStringList &filesList,
                 const QString &file,
                 const QVariantList &changes);

    //void loadingItem(void){showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在加载..."));}

    void process();

    void ok(uchar state);

    void kk(bool v);

private:
    void loadViewItems(const QVariantList &recordsList, ViewType view);

    void countLocations(PictureGraphicsScene::SceneType type);

    Ui::TaskPageWidget *ui;
    int m_tabId;
    bool m_collapsed, m_changed;

    PreviewDialog *m_previewDlg;
    LoadingDlg *m_loadingDlg;
    LoadingDlg1 *m_loadingDlg1;

    QSettings m_Settings;
    FileParser m_taskParser;
    LoaderThread *m_photosLoader, *m_templatesLoader, *m_albumsLoader;
    //TaskPage::MakerThread m_maker;
    QStringList m_pictures;

    GraphicsScenesVector m_scensVector;
    PictureGraphicsScene *m_photosScene, *m_templatesScene, *m_albumsScene, *m_focusScene;

    /* Configurations */
    QVariantList m_photosList, m_templatesList, m_albumsList;

    /* Child windows/widgets */
    TemplatePageWidget *m_templatePage;
    EditPageWidget *m_editPage;

    //static QDialog *m_loadingDlg;

    friend class EditPageWidget;
};

class LoadingDlg1 : public QDialog
{
public:
    LoadingDlg1(TaskPageWidget *page);

    void showProcess(bool show,
                     QRect global = QRect(0, 0, 0, 0),
                     const QString &info = QString());

protected:
    void closeEvent(QCloseEvent *)
    {
        m_maker.stop();
    }

private:
    QLabel *m_movieLabel, *m_textLabel;
    TaskPageWidget *m_page;
    TaskPage::MakerThread /***/m_maker;
};

#endif // ALBUMITEMWIDGET_H
