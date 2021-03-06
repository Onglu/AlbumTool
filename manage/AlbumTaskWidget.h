﻿#ifndef ALBUMTASKWIDGET_H
#define ALBUMTASKWIDGET_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include "defines.h"

#ifndef PUBLIC_ENV
#define PUBLIC_ENV
#endif

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

    // 设置相册名称
    void setName(const QString &name);

    // 获取相册名称
    QString getName(void) const;

    // 获取相册UUID
    QString getUuid(void) const {return m_uuid;}

    // 获取相册MD5
    QString getMd5(void) const {return m_md5;}

    // 获取相册页数
    int getPagesNum(void) const {return m_pagesNum;}

    // 获取相册照片数
    int getPhotosNum(void) const {return m_photosNum;}

    // 获取相册空位数
    int getBlankNum(void) const {return m_blankNum;}

    // 获取相册大小
    quint64 getSize(void) const {return m_totalBytes;}

    // 设置相册可访问用户
    void setUsers(const UserInfoItems &users)
    {
        if (m_users != users)
        {
            m_users = users;
        }
    }

    // 获取相册可访问用户
    const UserInfoItems &getUsers(void){return m_users;}

    // 获取相册可访问用户的ID
    QString getUsersId(QString &ids);

    // 获取相册ID
    int getAlbumId(void) const {return m_aid;}

    // 获取相册类型
    uchar getAlbumType(void)
    {
        if (!m_atype)
        {
            m_atype = USER_ALBUM;
        }

        return m_atype;
    }

    // 获取相册所属影楼名称
    QString getBusinessName(void) const {return m_business.isEmpty() ? tr("无") : m_business;}

    // 设置相册的关联信息
    void setRelevance(uchar atype, const QString &business);

    // 获取相册的关联信息
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

    // 开始传输相册
    void start(int aid = 0);

    // 设置相册信息
    void setAlbumInfo(const QVariantMap &records);

    // 预览相册
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

    // 客户端提交动作
    bool postData(const QByteArray &binary, quint64 offset = 0);

    // 获取已上传大小
    void getUploadFileSize(void);

    // 加载相册记录信息
    void loadRecords(void);

    Ui::AlbumTaskWidget *ui;
    AlbumManageDialog *m_container;

    AlbumPictures m_pictures;
    UserInfoItems m_users;
    int m_pagesNum, m_photosNum, m_blankNum;

    // 传输管理监视对象
    QProcess m_tmaker;
    QTimer m_sender, m_watcher;
    QTime m_time;

    // 相册文件属性信息
    FileParser *m_file;
    QString m_name, m_uuid, m_md5, m_business;
    int m_tid, m_aid;
    quint64 m_totalBytes, m_sentBytes, m_readBytes, m_finishBytes;
    TaskState m_state;
    uchar m_atype;
    bool m_changed;

    // 传输管理控制对象
    QHttp *m_http;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    int m_error, m_times;
};

#endif // ALBUMTASKWIDGET_H
