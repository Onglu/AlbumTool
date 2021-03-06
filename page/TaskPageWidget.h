#ifndef ALBUMITEMWIDGET_H
#define ALBUMITEMWIDGET_H

#include <QWidget>
#include <QSettings>
#include "parser/FileParser.h"
#include "page/EditPageWidget.h"
#include "wrapper/PictureGraphicsScene.h"

class LoadingDialog;
class PreviewDialog;
class TemplateChildWidget;
class TaskPageWidget;

using namespace TaskLoader;

namespace TaskPage
{
    class MakerThread : public QThread
    {
        Q_OBJECT

    public:
        MakerThread(void) : m_abort(false){}

        void begin(const QStringList &args)
        {
            m_args = args;
            m_abort = false;
            start();
        }

        void end(){m_abort = true;}

    signals:
        void doing(int index, const QStringList &args);

    protected:
        void run()
        {
            int index = 0;

            while (!m_abort)
            {
                if (!m_args.isEmpty())
                {
                    emit doing(++index, m_args);
                    msleep(20);
                }
            }
        }

    private:
        bool m_abort;
        QStringList m_args;
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

    bool hasOpened(const QString &taskFile) const
    {
        return (taskFile == QDir::toNativeSeparators(m_taskParser.fileName()));
    }

    bool isEditing(void) const {return m_editPage->isVisible();}

    void setTabId(int tabId){m_tabId = tabId;}
    void importTemplates(void);

    void onPreview(const QStringList &pictures, int current);
    void onEdit(const ChildWidgetsMap &albumsMap, int current);
    void onSearch(const QVariantMap &tags = QVariantMap());

    bool hasChanged(void) const {return m_changed;}
    char *saveFile(uchar mode);
    void saveChanges(void);
    void noticeChanged(void);

    bool replace(PictureGraphicsScene::SceneType type,
                 const QString &current,
                 const QString &replaced);

    TemplateChildWidget *getTmplWidget(const QString &tmplFile) const;

    EditPageWidget *getEditPage(void) const {return m_editPage;}

    QString getPageId(void) const {return m_taskParser.getPageId();}

    QString getAlbumFile(void);

signals:
    void changed(int index);

protected:
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void customEvent(QEvent *);

private slots:
    void on_collapsePushButton_clicked();

    void on_importPhotosPushButton_clicked();

    void on_addAlbumPushButton_clicked();

    void on_fillPushButton_clicked();

    void on_createPushButton_clicked();

    void on_previewPushButton_clicked();

    void detachItem(QGraphicsScene *scene, const QString &file);

    void updateViews(void);

    void enterEdit(bool enter);

    void addItem(int index, const QString &file, qreal angle, Qt::Axis axis, int usedTimes);

    void addItem(int index, const QString &file, int usedTimes);

    void addItem(int index,
                 const QVariantList &filesList,
                 const QString &file,
                 const QVariantList &changes);

    void process(int index, const QStringList &args);

    void ok(uchar state);

private:
    void loadViewItems(const QVariantList &recordsList, ViewType view);

    void countLocations(PictureGraphicsScene::SceneType type);

    void addAlbumRecord(int pagesNum, int photosNum, int blankNum);

    Ui::TaskPageWidget *ui;
    int m_tabId;    // tab page id
    bool m_collapsed, m_changed;

    PreviewDialog *m_previewDlg;
    LoadingDialog *m_loadingDlg;

    QSettings m_Settings;
    FileParser m_taskParser;
    LoaderThread *m_photosLoader, *m_templatesLoader, *m_albumsLoader;
    TaskPage::MakerThread m_maker;
    QString m_album;
    int m_photosNum;

    QVariantMap m_package;
    QStringList m_pictures;

    GraphicsScenesVector m_scensVector;
    PictureGraphicsScene *m_photosScene, *m_templatesScene, *m_albumsScene, *m_focusScene;
    ProxyWidgetsMap m_availablePhotos, m_availableAlbums;

    /* Configuration records */
    QVariantList m_photosList, m_templatesList, m_albumsList, m_pages;

    /* Child windows/widgets */
    TemplatePageWidget *m_templatePage;
    EditPageWidget *m_editPage;
    QTime m_tm;

    friend class EditPageWidget;
};

#endif // ALBUMITEMWIDGET_H
