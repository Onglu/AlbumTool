#ifndef ALBUMITEMWIDGET_H
#define ALBUMITEMWIDGET_H

#include <QWidget>
#include <QSettings>
#include "parser/FileParser.h"
#include "page/EditPageWidget.h"
#include "wrapper/utility.h"
#include "wrapper/PictureGraphicsScene.h"

class PreviewDialog;
class TemplateChildWidget;

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
    bool isEditing(void) const {return m_pEditPage->isVisible();}

    void setTabId(int tabId){m_tabId = tabId;}
    void importTemplate(void);

    void onPreview(const QStringList &pictures, int current);
    void onEdit(const ChildWidgetsMap &albumsMap, int current);
    void onSearch(bool immediate, bool inner, const QVariantMap &tags);

    bool hasChanged(void) const {return m_bChanged;}
    char *saveFile(uchar mode);
    void saveChanges(void);
    void noticeChanged(void);

    bool replace(PictureGraphicsScene::SceneType type,
                 const QString &current,
                 const QString &replaced);

    TemplateChildWidget *getTemplateWidget(const QString &tmplFile) const;

    static void showProcess(bool show,
                            QRect global = QRect(0, 0, 0, 0),
                            const QString &info = QString());

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

    void detachItem(QGraphicsScene *scene, const QString &file);

    void updateViews(void);

    void enterEdit(bool enter);

    void addItem(int index, const QString &file, qreal angle, Qt::Axis axis, int usedTimes);

    void addItem(int index, const QString &file, int usedTimes);

    void addItem(int index, const QStringList &filesList, const QString &file);

    void loadingItem(void){showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在加载..."));}

    void finishLoaded(uchar state);

private:
    void loadViewItems(const QVariantList &recordsList, ViewType view);

    Ui::TaskPageWidget *ui;
    int m_tabId;
    bool m_bCollapsed, m_bChanged;

    QSettings m_Settings;
    FileParser m_taskParser;
    LoaderThread *m_pPhotosLoader, *m_pTemplatesLoader, *m_pAlbumsLoader;

    GraphicsScenesVector m_scensVector;
    PictureGraphicsScene *m_pPhotosScene, *m_pTemplatesScene, *m_pAlbumsScene, *m_pFocusScene;

    /* Configurations list */
    QVariantList m_photosList, m_templatesList, m_albumsList;

    /* Child windows/widgets */
    PreviewDialog *m_pPreviewDlg;
    TemplatePageWidget *m_pTemplatePage;
    EditPageWidget *m_pEditPage;

    static QDialog *m_pLoadingDlg;

    friend class EditPageWidget;
};

#endif // ALBUMITEMWIDGET_H
