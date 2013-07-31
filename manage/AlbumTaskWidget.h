#ifndef ALBUMTASKWIDGET_H
#define ALBUMTASKWIDGET_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include "defines.h"

#define MAX_TIMEOUT                 30000
#define USER_ALBUM                  1
#define SAMPLE_ALBUM                2

class FileParser;
class QHttp;
class QNetworkAccessManager;
class AlbumManageDialog;
typedef QList<QPixmap> AlbumPictures;
typedef QHash<int, QString> UserInfoItems;

namespace Ui {
class AlbumTaskWidget;
}

class AlbumTaskWidget : public QWidget
{
    Q_OBJECT
    
public:
    AlbumTaskWidget(const QString &album, AlbumManageDialog *container);
    ~AlbumTaskWidget();

    enum TaskState{Pause, Initialize, Start, Finished, Published};
    TaskState getState(void) const {return m_state;}

    void setName(const QString &name);

    QString getName(void) const;

    QString getUuid(void) const {return m_uuid;}

    QString getMd5(void) const {return m_md5;}

    int getPagesNum(void) const {return m_pagesNum;}

    int getPhotosNum(void) const {return m_photosNum;}

    int getBlankNum(void) const {return m_blankNum;}

    quint64 getSize(void) const {return m_totalBytes;}

    void setUsers(const UserInfoItems &users)
    {
        if (m_users != users)
        {
            m_users = users;
        }
    }

    const UserInfoItems &getUsers(void){return m_users;}

    QString getUsersId(QString &ids);

    int getAlbumId(void) const {return m_aid;}

    uchar getAlbumType(void)
    {
        if (!m_atype)
        {
            m_atype = USER_ALBUM;
        }

        return m_atype;
    }

    QString getBusinessName(void) const {return m_business.isEmpty() ? tr("无") : m_business;}

    void setRelevance(uchar atype, const QString &business);

    void getRelevance(uchar &atype, QString &business)
    {
        atype = m_atype;

        if (m_business.isEmpty())
        {
            business = tr("无");
        }
        else
        {
            business = m_business;
        }
    }

    void start(int aid = 0);

    void setAlbumInfo(const QVariantMap &records);

    void onPreview(void);

protected:
    void enterEvent(QEvent *){moveOver();}
    void leaveEvent(QEvent *){moveOver();}
    
private slots:
    void processFinished(int, QProcess::ExitStatus);

    void end();

    void over();

    void replyError(QNetworkReply::NetworkError error){m_error = error;}

    void replyFinished(QNetworkReply *reply);

    void on_actionPushButton_clicked();

    void on_editPushButton_clicked();

    void on_nameLineEdit_editingFinished();

    void on_viewPushButton_clicked();

private:
    void moveOver();

    bool postData(const QByteArray &binary, quint64 offset = 0);

    void getUploadFileSize(void);

    void loadRecords(void);

    Ui::AlbumTaskWidget *ui;
    AlbumManageDialog *m_container;

    AlbumPictures m_pictures;
    UserInfoItems m_users;
    int m_pagesNum, m_photosNum, m_blankNum;

    QProcess m_tmaker;
    QTimer m_sender, m_watcher;
    QTime m_time;

    FileParser *m_file;
    QString m_name, m_uuid, m_md5, m_business;
    int m_tid, m_aid;
    quint64 m_totalBytes, m_sentBytes, m_readBytes;
    TaskState m_state;
    uchar m_atype;
    bool m_changed;

    QHttp *m_http;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    int m_error, m_times;
};

#endif // ALBUMTASKWIDGET_H
