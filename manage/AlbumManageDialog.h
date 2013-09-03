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

    // 打开相册管理窗口
    void openWnd(const QStringList &albums = QStringList());

    // 添加相册任务
    void addTask(const QString &album);

    // 更新相册列表
    void updateList(void);

    // 获取相册列表
    const QStringList &getAlbumsList(void) const {return m_albumsList;}

    // 设置相册信息
    void setAlbumInfo(AlbumTaskWidget &task, bool view = false);

    // 更具电话号码查找用户
    void findUser(const QString &telephone);

    // 添加用户
    void addUser(const QString &telephone,
                 const QString &realname,
                 uchar sex = 0);

    // 获取用户key
    QString getUserKey(void) const {return m_userKey;}

    // 获取用户ID
    int getUserId(void) const {return m_uid;}

    // 获取商家ID
    int getBusinessId(const QString &name) const;

    // 获取商家名称
    QString getBusinessName(int id, QString &name) const;

    // 获取商家列表
    const QStringList &getBusinesses(QStringList &businesses) const;

    // 获取当前相册任务
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
    // 切换相册任务列表
    void switchPage(void);

    Ui::AlbumManageDialog *ui;

    // 相册管理页的基本控件
    AlbumInfoWidget *m_setPage, *m_editPage;
    AlbumTaskWidget *m_currTask;

    // 相册任务列表及用户信息
    WidgetItemsList m_itemsList;
    QStringList m_albumsList;
    QVariantList m_businessesList;
    QString m_url, m_userKey;

    int m_uid;
    bool m_logined;

    // 传输管理控制对象
    QHttp *m_http;
    QNetworkAccessManager *m_manager;

    QTimer m_watcher;
    LoadingDialog *m_loadingDlg;
};

#endif // ALBUMMANAGEDIALOG_H
