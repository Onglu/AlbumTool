#ifndef ALBUMMANAGEDIALOG_H
#define ALBUMMANAGEDIALOG_H

#include <QDialog>
#include <QVariantList>
#include <QTimer>

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

    void setInfo(AlbumTaskWidget &album);

    QString getUserKey(void) const {return m_userKey;}

    int getUserId(void) const {return m_uid;}

    int getBusinessId(const QString &name) const;
    
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

    void finish(bool ok);

private:
    void switchPage(void);

    void createAlbum(int bid, bool sample = false);

    Ui::AlbumManageDialog *ui;

    AlbumInfoWidget *m_info;
    AlbumTaskWidget *m_album;

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
