#include "AlbumTaskWidget.h"
#include "ui_AlbumTaskWidget.h"
#include "AlbumManageDialog.h"
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
#define UPLOAD_FILE_URL             "http://192.168.2.120:8080/SwfUpload2/fileupload"
#define GET_UPLOAD_FILE_SIZE_URL    "http://192.168.2.120:8080/SwfUpload2/getuploadedsize?md5="

using namespace QtJson;

AlbumTaskWidget::AlbumTaskWidget(const QString &album, AlbumManageDialog *container) :
    QWidget(0),
    ui(new Ui::AlbumTaskWidget),
    m_container(container),
    m_tid(0),
    m_aid(0),
    m_atype(USER_ALBUM),
    m_state(Initialize),
    m_totalBytes(0),
    m_sentBytes(0),
    m_file(new FileParser(album))
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    ui->nameLineEdit->hide();
    ui->progressBar->hide();

    QString name;
    ui->nameLabel->setText(Converter::getFileName(album, name, false));

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

    connect(&m_sender, SIGNAL(timeout()), SLOT(end()));
    connect(&m_watcher, SIGNAL(timeout()), SLOT(over()));

    connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));

    TemplateChildWidget::useZip(m_tmaker,
                                TemplateChildWidget::ZipUsageRead,
                                tr("\"%1\" cover.png").arg(album));

    loadRecords();
}

AlbumTaskWidget::~AlbumTaskWidget()
{
    if (m_sender.isActive())
    {
        m_sender.stop();
        m_state = Pause;
    }

    if (m_tid && m_aid)
    {
        QSqlQuery query;
        QString sql = tr("update albums_upload_record set albumid=%1, name='%2', state=%3, finishsize=%4, type=%5 where id=%6").arg(m_aid).arg(ui->nameLabel->text()).arg(m_state).arg(m_sentBytes).arg(m_atype).arg(m_tid);
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
        //qDebug() << __FILE__ << __LINE__ << name;

        QPixmap pix(name);
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

        QFile::remove(name);
    }
}

void AlbumTaskWidget::loadRecords()
{
    if (!m_file || !m_file->isValid())
    {
        qDebug() << __FILE__ << __LINE__ << "album package is invalid!";
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
    QString sql = tr("select fileguid,pagesnum,photosnum from album_info where fileurl='%1'").arg(album);

    m_uuid.clear();

    query.exec(sql);
    while (query.next())
    {
        m_uuid = query.value(0).toString();
        m_pagesNum = query.value(1).toInt();
        m_photosNum = query.value(2).toInt();
    }

    if (m_uuid.isEmpty())
    {
        return;
    }

    m_file->getFileMd5(m_md5);

    sql = tr("select id,albumid,name,finishsize,filesize,state,type from albums_upload_record where fileurl='%1' and filemd5='%2'").arg(album).arg(m_md5);
    query.exec(sql);
    while (query.next())
    {
        m_tid = query.value(0).toInt();
        m_aid = query.value(1).toInt();
        ui->nameLabel->setText(query.value(2).toString());
        m_sentBytes = query.value(3).toULongLong();
        m_totalBytes = query.value(4).toULongLong();
        m_state = (TaskState)query.value(5).toInt();
        m_atype = (uchar)query.value(6).toInt();

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
        }
        else if (Published == m_state)
        {
            ui->stateLabel->setText(tr("已完成发布"));
            ui->actionPushButton->setText(tr("发布"));
            ui->actionPushButton->setEnabled(false);
        }
        //qDebug() << __FILE__ << __LINE__ << m_state;
    }

    //qDebug() << __FILE__ << __LINE__ << sql << m_tid;
    if (m_tid)
    {
        return;
    }

    sql = tr("select id,albumid,name,type from albums_upload_record where fileurl='%1'").arg(album);
    query.exec(sql);
    while (query.next())
    {
        m_tid = query.value(0).toInt();
        m_aid = query.value(1).toInt();
        ui->nameLabel->setText(query.value(2).toString());
        m_atype = (uchar)query.value(3).toInt();
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

void AlbumTaskWidget::start(int aid, uchar atype)
{
    if (aid)
    {
        m_aid = aid;
    }

    if (USER_ALBUM <= atype && SAMPLE_ALBUM >= atype)
    {
        m_atype = atype;
    }

    qDebug() << __FILE__ << __LINE__ << m_aid << m_atype;

    m_error = m_times = 0;
    m_sentBytes = m_readBytes = 0;

    ui->stateLabel->setText(tr("正在等待上传..."));
    ui->progressBar->show();
    ui->progressBar->setMaximum(m_totalBytes);
    ui->progressBar->setValue(m_sentBytes);
    ui->actionPushButton->setEnabled(false);

    m_sender.start(INTERVAL_TIME);
    m_watcher.start(1000);
    m_time.start();
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
            m_container->setInfo(*this);
        }
        else
        {
            start();
        }
        break;
    case Start:
        m_state = Pause;
        m_sender.stop();
        m_watcher.stop();
        ui->stateLabel->setText(tr("已暂停"));
        ui->actionPushButton->setText(tr("继续"));
        break;
    case Finished:
        break;
    case Published:
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
        m_time.restart();

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

void AlbumTaskWidget::replyFinished(QNetworkReply *reply)
{
    QByteArray ba = reply->readAll();
    QVariantMap result = QtJson::parse(QString(ba)).toMap();
    int code = result["protocol"].toInt();

    m_error = reply->error();
    //qDebug() << __FILE__ << __LINE__ << m_error << ba;
    reply->deleteLater();

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
        }
        else
        {
            if (m_readBytes)
            {
                float passed = (float)m_time.elapsed() / 1000;
                float time = (m_totalBytes - m_sentBytes) / m_readBytes * passed;
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

                //qDebug() << __FILE__ << __LINE__ << passed << time << units << m_readBytes / 1024 / passed;
                ui->stateLabel->setText(tr("正在上传（传输速度 %1 KB/s，剩余时间 %2 %3）").arg(m_readBytes / 1024 / passed).arg(time).arg(units));
                //ui->stateLabel->setText(tr("传输进度 %1 KB / %2 KB").arg(m_sentBytes / 1024).arg(m_totalBytes / 1024));
            }

            m_state = Start;
            ui->progressBar->setValue(m_sentBytes);
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
    QString text = ui->nameLineEdit->text();
    if (!text.isEmpty() && ui->nameLabel->text() != text)
    {
        ui->nameLabel->setText(text);
    }

    ui->nameLabel->show();
    ui->nameLineEdit->hide();
}

QString AlbumTaskWidget::getName() const
{
    return ui->nameLabel->text();
}
