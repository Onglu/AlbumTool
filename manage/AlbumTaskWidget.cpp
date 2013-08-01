#include "AlbumTaskWidget.h"
#include "ui_AlbumTaskWidget.h"
#include "AlbumManageDialog.h"
#include "AlbumInfoWidget.h"
#include "PreviewDialog.h"
#include "wrapper/utility.h"
#include "parser/FileParser.h"
#include "parser/json.h"
#include "child/TemplateChildWidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QtSql>
#include <QDate>
#include <QNetworkAccessManager>
#include <QHttp>

#define INTERVAL_TIME               100     // ms
#define FILE_DATA_CHUNK             /*1024*/100 * 1024
#define UPDATE_NAME_URL             "http://192.168.2.120:8080/SwfUpload2/updatename?"
#define UPLOAD_FILE_URL             "http://192.168.2.120:8080/SwfUpload2/fileupload"
#define GET_UPLOAD_FILE_SIZE_URL    "http://192.168.2.120:8080/SwfUpload2/getuploadedsize?md5="
#define PUBLISH_URL                 "http://192.168.2.120:8080/SwfUpload2/release?"

using namespace QtJson;

AlbumTaskWidget::AlbumTaskWidget(const QString &album, AlbumManageDialog *container) :
    QWidget(0),
    ui(new Ui::AlbumTaskWidget),
    m_container(container),
    m_file(new FileParser(album)),
    m_pagesNum(0),
    m_photosNum(0),
    m_blankNum(0),
    m_tid(0),
    m_aid(0),
    m_atype(USER_ALBUM),
    m_state(Initialize),
    m_totalBytes(0),
    m_sentBytes(0),
    m_changed(false)
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    ui->nameLineEdit->hide();
    ui->progressBar->hide();

    QString name;
    ui->nameLabel->setText(Converter::getFileName(album, name, false));

    loadRecords();

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

    connect(&m_sender, SIGNAL(timeout()), SLOT(end()));
    connect(&m_watcher, SIGNAL(timeout()), SLOT(over()));

    connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));

    TemplateChildWidget::useZip(m_tmaker,
                                TemplateChildWidget::ZipUsageRead,
                                tr("\"%1\" cover.png").arg(album));
}

AlbumTaskWidget::~AlbumTaskWidget()
{
    if (m_sender.isActive())
    {
        m_sender.stop();
        m_state = Pause;
    }

    if (m_changed && m_tid && m_aid)
    {
        QSqlQuery query;
        QString sql = tr("update albums_upload_record set albumid=%1, name='%2', state=%3, finishsize=%4, type=%5, business='%6' where id=%7").arg(m_aid).arg(ui->nameLabel->text()).arg(m_state).arg(m_sentBytes).arg(m_atype).arg(m_business).arg(m_tid);
        query.exec(sql);
        qDebug() << __FILE__ << __LINE__ << sql;
    }

    if (m_file)
    {
        m_file->closeFile();
        delete m_file;
    }

    delete ui;
}

void AlbumTaskWidget::moveOver()
{
    QIcon icon;
    QString text;

    if (ui->viewPushButton->text().isEmpty())
    {
        text = tr("查看");
        icon = QIcon(QPixmap(":images/edit.png"));
    }

    ui->viewPushButton->setText(text);
    ui->editPushButton->setIcon(icon);
}

void AlbumTaskWidget::onPreview()
{
    if (!m_pagesNum)
    {
        return;
    }

    if (m_pagesNum != m_pictures.size() - 1)
    {
        QString album = QDir::toNativeSeparators(m_file->fileName());
        for (int i = 1; i <= m_pagesNum; i++)
        {
            TemplateChildWidget::useZip(m_tmaker,
                                        TemplateChildWidget::ZipUsageRead,
                                        tr("\"%1\" page%2.png").arg(album).arg(i));
        }
    }
    else
    {
        PreviewDialog dlg(this);
        dlg.updateList(m_pictures);
        dlg.exec();
    }
}

void AlbumTaskWidget::processFinished(int ret, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(ret);

    if (QProcess::CrashExit == exitStatus)
    {
        QMessageBox::critical(this, tr("解析失败"), tr("错误信息：tmaker.exe has crashed!"), tr("确定"));
        return;
    }

    QString content(m_tmaker.readAllStandardOutput());
    m_tmaker.close();

    if (content.startsWith("picture:"))
    {
        QString name = content.mid(8);
        QPixmap pix(name);

        if (!ui->coverLabel->pixmap())
        {
            if (!pix.isNull())
            {
                ui->coverLabel->setPixmap(pix.scaled(ui->coverLabel->size(),
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));
            }
            else
            {
                ui->coverLabel->setText(tr("无法显示"));
            }
        }

        if (!pix.isNull())
        {
            m_pictures << pix;
            if (m_pagesNum && m_pagesNum == m_pictures.size() - 1)
            {
                PreviewDialog dlg(this);
                dlg.updateList(m_pictures);
                dlg.exec();
            }
        }

        QFile::remove(name);
    }
}

void AlbumTaskWidget::loadRecords()
{
    if (!m_file || !m_file->isValid())
    {
        qDebug() << __FILE__ << __LINE__ << "album package is invalid!";
        return;
    }

    SqlHelper sh;
    if (!sh.connectDb())
    {
        qDebug() << __FILE__ << __LINE__ << "error:" << sh.getError();
        return;
    }

    if (!m_file->openFile(QIODevice::ReadOnly))
    {
        return;
    }

    QSqlQuery query;
    QString album = QDir::toNativeSeparators(m_file->fileName());
    QString sql = tr("select fileguid,pagesnum,photosnum,blanknum from album_info where fileurl='%1'").arg(album);

    m_uuid.clear();

    query.exec(sql);
    while (query.next())
    {
        m_uuid = query.value(0).toString();
        m_pagesNum = query.value(1).toInt();
        m_photosNum = query.value(2).toInt();
        m_blankNum = query.value(3).toInt();
    }

    if (m_uuid.isEmpty())
    {
        return;
    }

    m_file->getFileMd5(m_md5);

    sql = tr("select id,albumid,name,finishsize,filesize,state,type,business from albums_upload_record where fileurl='%1' and filemd5='%2'").arg(album).arg(m_md5);
    query.exec(sql);
    while (query.next())
    {
        m_tid = query.value(0).toInt();
        m_aid = query.value(1).toInt();
        ui->nameLabel->setText(query.value(2).toString());
        m_sentBytes = query.value(3).toULongLong();
        m_totalBytes = query.value(4).toULongLong();
        m_state = (TaskState)query.value(5).toInt();

        if (Pause == m_state)
        {
            ui->stateLabel->setText(tr("已暂停"));
            ui->actionPushButton->setText(tr("继续"));
            ui->progressBar->show();
            ui->progressBar->setMaximum(m_totalBytes);
            ui->progressBar->setValue(m_sentBytes);
        }
        else if (Finished == m_state)
        {
            ui->stateLabel->setText(tr("已完成上传"));
            ui->actionPushButton->setText(tr("发布"));
            ui->editPushButton->setEnabled(true);
            ui->viewPushButton->setEnabled(true);
        }
        else if (Published == m_state)
        {
            ui->stateLabel->setText(tr("已完成发布"));
            ui->actionPushButton->setText(tr("发布"));
            ui->actionPushButton->setEnabled(false);
            ui->editPushButton->setEnabled(true);
            ui->viewPushButton->setEnabled(true);
        }

        m_atype = (uchar)query.value(6).toInt();
        if (!m_atype)
        {
            m_atype = USER_ALBUM;
        }

        m_business = query.value(7).toString();
    }

    //qDebug() << __FILE__ << __LINE__ << m_atype;
    if (m_tid)
    {
        return;
    }

    sql = tr("select id,albumid,name,type,business from albums_upload_record where fileurl='%1'").arg(album);
    query.exec(sql);
    while (query.next())
    {
        m_tid = query.value(0).toInt();
        m_aid = query.value(1).toInt();
        ui->nameLabel->setText(query.value(2).toString());
        m_atype = (uchar)query.value(3).toInt();
        if (!m_atype)
        {
            m_atype = USER_ALBUM;
        }

        m_business = query.value(4).toString();
    }

    m_totalBytes = m_file->size();

    if (!m_tid)
    {
        sql = tr("insert into albums_upload_record(name,createtime,fileurl,filemd5,filesize) values('%1','%2','%3','%4',%5)").arg(ui->nameLabel->text()).arg(QDate::currentDate().toString("yyyy-MM-dd")).arg(album).arg(m_md5).arg(m_totalBytes);
        query.exec(sql);
        m_tid = query.lastInsertId().toInt();
    }
    else
    {
        sql = tr("update albums_upload_record set createtime='%1', filemd5='%2', finishsize=0, filesize=%3, state=1 where id=%4").arg(QDate::currentDate().toString("yyyy-MM-dd")).arg(m_md5).arg(m_totalBytes).arg(m_tid);
        query.exec(sql);
    }

    //qDebug() << __FILE__ << __LINE__ << sql << m_tid;
}

void AlbumTaskWidget::setRelevance(uchar atype, const QString &business)
{
    if (USER_ALBUM <= atype && SAMPLE_ALBUM >= atype)
    {
        m_atype = atype;
    }

    m_business = business;
}

void AlbumTaskWidget::start(int aid)
{
    qDebug() << __FILE__ << __LINE__ << m_aid << aid;

    if (0 < aid)
    {
        m_aid = aid;
    }

    if (Initialize == m_state)
    {
        m_error = m_times = 0;
        m_sentBytes = m_readBytes = m_finishBytes = 0;

        ui->stateLabel->setText(tr("正在等待上传..."));
        ui->progressBar->show();
        ui->progressBar->setMaximum(m_totalBytes);
        ui->progressBar->setValue(m_sentBytes);
        ui->editPushButton->setEnabled(false);
        ui->actionPushButton->setEnabled(false);
        ui->viewPushButton->setEnabled(false);

        m_sender.start(INTERVAL_TIME);
        m_watcher.start(1000);
        m_time.start();
    }
}

void AlbumTaskWidget::on_actionPushButton_clicked()
{
    switch (m_state)
    {
    case Pause:
        ui->stateLabel->setText(tr("正在等待上传..."));
        ui->actionPushButton->setEnabled(false);
        getUploadFileSize();
        break;
    case Initialize:
        if (!m_aid)
        {
            m_container->setAlbumInfo(*this);
        }
        else
        {
            start();
        }
        break;
    case Start:
        m_finishBytes = 0;
        m_state = Pause;
        m_sender.stop();
        m_watcher.stop();
        ui->stateLabel->setText(tr("已暂停"));
        ui->actionPushButton->setText(tr("继续"));
        break;
    case Finished:
        {
            QUrl url(tr("%1userkey=%2&employeeid=%3&id=%4&type=%5").arg(PUBLISH_URL).arg(m_container->getUserKey()).arg(m_container->getUserId()).arg(m_aid).arg(m_atype));
            QNetworkRequest request(url);
            if (m_manager->get(request))
            {
                ui->editPushButton->setEnabled(false);
                ui->actionPushButton->setEnabled(false);
                ui->viewPushButton->setEnabled(false);
            }
        }
        break;
    }
}

void AlbumTaskWidget::getUploadFileSize()
{
    QNetworkRequest request(QUrl(GET_UPLOAD_FILE_SIZE_URL + m_md5));
    QNetworkReply *reply = m_manager->get(request);
    if (reply)
    {
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
    }
}

bool AlbumTaskWidget::postData(const QByteArray &binary, quint64 offset)
{
    if (m_md5.isEmpty() || binary.isEmpty() || offset >= m_totalBytes)
    {
        return false;
    }

    QString boundary = QString("--------------------------%1%2%3").arg(qrand()).arg(qrand()).arg(qrand());
    QString startBoundary = "--" + boundary + "\r\n";
    QString contentType = "multipart/form-data; boundary=" + boundary;

    QByteArray data;
    data.append("\r\n");

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"userkey\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(m_container->getUserKey()));

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"employeeid\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(m_container->getUserId()));

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"id\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(m_aid));

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"type\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(m_atype));

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"size\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(m_totalBytes));

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"offset\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(offset));

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"md5\"\r\n\r\n");
    data.append(m_md5 + "\r\n");

    data.append(startBoundary);
    data.append("Content-Disposition: form-data; name=\"files\"\r\n\r\n");
    data.append(binary);
    data.append("\r\n");
    data.append("--" + boundary + "--\r\n");

    //qDebug() << __FILE__ << __LINE__ << binary.length();//"data:" << data;

    QNetworkRequest request(QUrl(UPLOAD_FILE_URL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType.toAscii());
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(data.size()).toString());

    QNetworkReply *reply = m_manager->post(request, data);
    if (reply)
    {
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
        return true;
    }

    return false;
}

void AlbumTaskWidget::end()
{
    bool stop = false;

    if (0 > m_error)
    {
        return; // Waitting...
    }
    else if (!m_error)
    {
        //m_time.restart();

        if (FILE_DATA_CHUNK >= m_totalBytes)
        {
            postData(m_file->readAll());
        }
        else
        {
            char *buf = new char[FILE_DATA_CHUNK + 1];
            memset(buf, 0, FILE_DATA_CHUNK + 1);

            m_file->seek(m_sentBytes);
            m_readBytes = m_file->read(buf, FILE_DATA_CHUNK);

            if (postData(QByteArray::fromRawData(buf, m_readBytes), m_sentBytes))
            {
                m_sentBytes += m_readBytes;
                if (m_totalBytes == m_sentBytes)
                {
                    stop = true;
                    m_watcher.stop();
                }
                else
                {
                    m_error = -1;
                }
            }

            //qDebug() << __FILE__ << __LINE__ << m_readBytes << "," << m_sentBytes << "/" << m_totalBytes;

            delete []buf;
            buf = NULL;
        }
    }
    else
    {
        stop = true;
    }

    if (stop)
    {
        //qDebug() << __FILE__ << __LINE__ << m_error;
        //m_file->closeFile();
        m_finishBytes = 0;
        m_file->seek(0);
        m_sender.stop();
    }
}

void AlbumTaskWidget::over()
{
    int m = m_time.elapsed();

    if (MAX_TIMEOUT <= m && m < MAX_TIMEOUT + 1000)
    {
        qDebug() << __FILE__ << __LINE__ << "elapsed" << m << "ms";

        if (2 > ++m_times)
        {
            if (m_totalBytes != m_sentBytes && m_sentBytes >= m_readBytes)
            {
                m_error = 0;
                m_sentBytes -= m_readBytes;
                end();
            }
        }
        else
        {
            m_sender.stop();
            m_watcher.stop();
            ui->stateLabel->setText(tr("上传超时，请稍后重试！"));
            ui->actionPushButton->setEnabled(true);
            if (0 < ui->progressBar->value())
            {
                m_state = Pause;
                ui->actionPushButton->setText(tr("继续"));
            }
        }
    }
}

void AlbumTaskWidget::setAlbumInfo(const QVariantMap &value)
{
    if (value.isEmpty())
    {
        return;
    }

    m_atype = (uchar)value["type"].toInt();
    m_business = value["businessname"].toString();
    m_users.clear();

    QVariantList users = value["customerliset"].toList();
    foreach (const QVariant &user, users)
    {
        QVariantMap record = user.toMap();
        int id = record["id"].toInt();
        m_users[id] = tr(" 手机号码：%1\t\t姓名：%2\t\t初始密码：%3").arg(record["telephone"].toString()).arg(record["realname"].toString()).arg(record["initpassword"].toString());
    }
}

void AlbumTaskWidget::replyFinished(QNetworkReply *reply)
{
    QByteArray ba = reply->readAll();
    QVariantMap result = QtJson::parse(QString(ba)).toMap();
    int code = result["protocol"].toInt();
    QString url = reply->url().toString();

    m_error = reply->error();
    //qDebug() << __FILE__ << __LINE__ << m_error << ba << url;
    reply->deleteLater();

    if (SERVER_REPLY_SUCCESS == code && !m_changed)
    {
        m_changed = true;
    }

    if (url.startsWith(UPDATE_NAME_URL))
    {
        if (SERVER_REPLY_SUCCESS != code)
        {
            QMessageBox::information(this, tr("更改失败"), tr("相册名称更改失败，错误码：%1").arg(code), tr("确定"));
        }
        else
        {
            ui->nameLabel->setText(m_name);
        }

        return;
    }
    else if (url.startsWith(PUBLISH_URL))
    {
        if (SERVER_REPLY_SUCCESS != code)
        {
            QMessageBox::information(this, tr("发布失败"), tr("相册发布失败，错误码：%1").arg(code), tr("确定"));
            ui->actionPushButton->setEnabled(true);
        }
        else
        {
            QMessageBox::information(this, tr("发布成功"), tr("本相册已经成功发布到服务器。"), tr("确定"));
            m_state = Published;
            ui->stateLabel->setText(tr("已完成发布"));
        }

        ui->editPushButton->setEnabled(true);
        ui->viewPushButton->setEnabled(true);

        return;
    }
    else
    {
        if (SERVER_REPLY_SUCCESS != code)
        {
            m_error = 1;
            return;
        }

        if (!ui->actionPushButton->isEnabled())
        {
            ui->actionPushButton->setEnabled(true);
            ui->actionPushButton->setText(tr("暂停"));
            m_container->updateList();
        }

        if (Pause == m_state)
        {
            QVariantMap value = result["retValue"].toMap();
            if (!value.isEmpty())
            {
                quint64 offset = value["size"].toULongLong();
                m_sentBytes = FILE_DATA_CHUNK < offset ? offset - FILE_DATA_CHUNK : offset;
                m_finishBytes = m_readBytes = 0;
                m_error = m_times = 0;
                m_state = Start;
                m_sender.start(INTERVAL_TIME);
                m_watcher.start(1000);
                m_time.start();
                qDebug() << __FILE__ << __LINE__ << "start position:" << m_sentBytes;
            }
        }
        else
        {
            if (m_totalBytes == m_sentBytes)
            {
                m_state = Finished;
                ui->stateLabel->setText(tr("已完成上传"));
                ui->progressBar->hide();
                ui->actionPushButton->setText(tr("发布"));
                m_container->updateList();
                ui->editPushButton->setEnabled(true);
                ui->viewPushButton->setEnabled(true);
            }
            else
            {
                float elapsed = (float)m_time.elapsed() / 1000;
                m_finishBytes += m_readBytes;
                if (m_finishBytes && (int)elapsed)
                {
                    int speed = m_finishBytes / 1024 / elapsed;
                    int time = (m_totalBytes - m_sentBytes) / m_finishBytes * elapsed;
                    QString units;

                    if (60 <= time)
                    {
                        time /= 60;
                        units = tr("分钟");
                    }
                    else
                    {
                        units = tr("秒");
                    }

                    //qDebug() << __FILE__ << __LINE__ << ":" << m_totalBytes - m_sentBytes << m_finishBytes << elapsed << speed << "KB/s " << time << units;
                    ui->stateLabel->setText(tr("正在上传（传输速度 %1 KB/s，剩余时间 %2 %3）").arg(speed).arg(time).arg(units));
                    ui->progressBar->setValue(m_sentBytes);
                    m_finishBytes = 0;
                    m_time.restart();
                }

                m_state = Start;
            }
        }
    }
}

void AlbumTaskWidget::on_editPushButton_clicked()
{
    ui->nameLabel->hide();
    ui->nameLineEdit->show();
    ui->nameLineEdit->setText(ui->nameLabel->text());
}

void AlbumTaskWidget::on_nameLineEdit_editingFinished()
{
    QString name = ui->nameLineEdit->text();
    if (!name.isEmpty() && ui->nameLabel->text() != name)
    {
        ui->nameLabel->setText(name);
        setName(name);
    }

    ui->nameLabel->show();
    ui->nameLineEdit->hide();
}

void AlbumTaskWidget::setName(const QString &name)
{
    if (name != ui->nameLabel->text() && !name.isEmpty())
    {
        m_name = name;
        QUrl url(tr("%1employeeid=%2&userkey=%3&name=%4&id=%5&type=%6").arg(UPDATE_NAME_URL).arg(m_container->getUserId()).arg(m_container->getUserKey()).arg(name).arg(m_aid).arg(m_atype));
        QNetworkRequest request(url);
        m_manager->get(request);
    }
}

QString AlbumTaskWidget::getName() const
{
    return ui->nameLabel->text();
}

QString AlbumTaskWidget::getUsersId(QString &ids)
{
    int i = 0;
    UserInfoItems::const_iterator iter = m_users.constBegin();

    ids.clear();

    while (iter != m_users.constEnd())
    {
        if (!i)
        {
            ids = QString("%1").arg(iter.key());
        }
        else
        {
            ids.append(QString(",%1").arg(iter.key()));
        }

        ++i;
        ++iter;
    }

    return ids;
}

void AlbumTaskWidget::on_viewPushButton_clicked()
{
    m_container->setAlbumInfo(*this, true);
}
