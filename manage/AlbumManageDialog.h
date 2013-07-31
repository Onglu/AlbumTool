#ifndef ALBUMMANAGEDIALOG_H
#define ALBUMMANAGEDIALOG_H

#include <QDialog>
#include <QVariantList>
#include <QTimer>

#define NO_RESULT                   9999
#define SERVER_REPLY_SUCCESS        10000
#define NO_SUCH_USER                10003
#define EXISTING_TELNO              10007
#define INVALID_TELNO               10015

class QHttp;
class QNetworkAccessManager;
class QNetworkReply;
class LoadingDialog;
class AlbumInfoWidget;
class AlbumTaskWidget;
class QListWidgetItem;
typedef QList<QListWidgetItem *> WidgetItemsList;

namespace Ui {
class AlbumManageDialog;
}

class AlbumManageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AlbumManageDialog(QWidget *parent = 0);
    ~AlbumManageDialog();

    void openWnd(const QStringList &albums = QStringList());

    void addTask(const QString &album);

    void updateList(void);

    const QStringList &getAlbumsList(void) const {return m_albumsList;}

    void setAlbumInfo(AlbumTaskWidget &task, bool view = false);

    void findUser(const QString &telephone);

    void addUser(const QString &telephone,
                 const QString &realname,
                 uchar sex = 0);

    QString getUserKey(void) const {return m_userKey;}

    int getUserId(void) const {return m_uid;}

    int getBusinessId(const QString &name) const;

    QString getBusinessName(int id, QString &name) const;

    const QStringList &getBusinesses(QStringList &businesses) const;

    AlbumTaskWidget *getCurrTask(void) const {return m_currTask;}

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_uploadPushButton_clicked(){switchPage();}

    void on_uploadingPushButton_clicked(){switchPage();}

    void on_unuploadPushButton_clicked(){switchPage();}

    void on_uploadedPushButton_clicked(){switchPage();}

    void on_loginPushButton_clicked();

    void over();

    void replyFinished(QNetworkReply *reply);

    void accept(bool ok, const QString &business, bool sample);

    void on_keywordLineEdit_textChanged(const QString &arg1);

    void on_searchPushButton_clicked();

private:
    void switchPage(void);

    Ui::AlbumManageDialog *ui;

    AlbumInfoWidget *m_setPage, *m_editPage;
    AlbumTaskWidget *m_currTask;

    WidgetItemsList m_itemsList;
    QStringList m_albumsList;
    QVariantList m_businessesList;
    QString m_url, m_userKey;

    int m_uid;
    bool m_logined;

    QHttp *m_http;
    QNetworkAccessManager *m_manager;

    QTimer m_watcher;
    LoadingDialog *m_loadingDlg;
};

#endif // ALBUMMANAGEDIALOG_H
