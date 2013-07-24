#ifndef ALBUMTASKWIDGET_H
#define ALBUMTASKWIDGET_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include "defines.h"

#define SERVER_REPLY_SUCCESS        10000
#define MAX_TIMEOUT                 30000
#define USER_ALBUM                  1
#define SAMPLE_ALBUM                2

class FileParser;
class QHttp;
class QNetworkAccessManager;
class AlbumManageDialog;

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

    QString getName(void) const;

    QString getUuid(void) const {return m_uuid;}

    QString getMd5(void) const {return m_md5;}

    int getPagesNum(void) const {return m_pagesNum;}

    int getPhotosNum(void) const {return m_photosNum;}

    quint64 getSize(void) const {return m_totalBytes;}

    void start(int aid = 0, uchar atype = 0);

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

private:
    void moveOver();

    bool postData(const QByteArray &binary, quint64 offset = 0);

    void getUploadFileSize(void);

    void loadRecords(void);

    Ui::AlbumTaskWidget *ui;
    AlbumManageDialog *m_container;

    QProcess m_tmaker;
    QTimer m_sender, m_watcher;
    QTime m_time;

    FileParser *m_file;
    QString m_uuid, m_md5;
    int m_pagesNum, m_photosNum, m_tid, m_aid;
    uchar m_atype;
    quint64 m_totalBytes, m_sentBytes, m_readBytes;
    TaskState m_state;

    QHttp *m_http;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    int m_error, m_times;
};

#endif // ALBUMTASKWIDGET_H
