#include "AlbumManageDialog.h"
#include "ui_AlbumManageDialog.h"
#include "parser/FileParser.h"
#include "defines.h"
#include <QDebug>
//#include <QNetworkRequest>
#include <QHttpRequestHeader>
#include <QHttp>

AlbumManageDialog::AlbumManageDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint),
    ui(new Ui::AlbumManageDialog)
{
    ui->setupUi(this);

    QUrl url(UPLOAD_URL);

    m_headr = new QHttpRequestHeader("POST", url.path());

    m_http = new QHttp(url.host(), url.port(), this);
    connect(m_http, SIGNAL(dataSendProgress(int,int)), SLOT(httpSendProgress(int,int)));
    connect(m_http, SIGNAL(requestFinished(int,bool)), SLOT(httpRequestFinished(int, bool)));

    connect(&m_timer, SIGNAL(timeout()), SLOT(end()));
}

AlbumManageDialog::~AlbumManageDialog()
{
    if (m_timer.isActive())
    {
        m_timer.stop();
    }

    delete ui;
}

void AlbumManageDialog::on_operatePushButton_clicked()
{
    QString fileName(tr("E:\\images\\album\\cover.png"));   // album.xc
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    QString md5;
    const QString boundary = QString("----------------------------%1%2%3%4\r\n").arg(qrand()).arg(qrand()).arg(qrand()).arg(qrand());

    QByteArray data;
    data.append("\r\n");

//    QHash<QString, QString>::ConstIterator i;
//    for (i = params.constBegin(); i != params.constEnd(); ++i)
//    {
//        ba.append( "--" + boundary + "\r\n");
//        ba.append("Content-Disposition: form-data; name=" + i.key() +"\r\n\r\n");
//        ba.append( i.value() +"\r\n");
//    }


    data.append(boundary);
    data.append("Content-Disposition: form-data; name=\"offset\"\r\n\r\n");
    data.append(QString("%1\r\n").arg(0));

    data.append(boundary);
    data.append("Content-Disposition: form-data; name=\"md5\"\r\n\r\n");
    data.append(FileParser::getFileMd5(fileName, md5) + "\r\n");

    data.append(boundary);
    data.append("Content-Disposition: form-data; name=\"files\"; filename=\"" + fileName + "\"\r\n");
    data.append("Content-Type: image/png\r\n\r\n");
    data.append(file.readAll());
    data.append("\r\n");
    data.append(boundary);

    m_headr->setContentType("multipart/form-data; boundary=" + boundary);
    m_headr->setContentLength(data.length());

    int requestID = m_http->request(*m_headr, data);

    qDebug() << __FILE__ << __LINE__ << "data:" << data;
    qDebug() << __FILE__ << __LINE__ << "requestID:" << requestID;

    file.close();
}

void AlbumManageDialog::on_testPushButton_clicked()
{
    m_timer.start(1000);
}

void AlbumManageDialog::httpSendProgress(int done, int total)
{
    ui->label->setText(QString("%1/%2").arg(done).arg(total));
}

void AlbumManageDialog::httpRequestFinished(int id, bool error)
{
    qDebug() << __FILE__ << __LINE__ << id << error << m_http->errorString();
}

void AlbumManageDialog::end()
{
    int v = ui->progressBar->value();
    const int m = ui->progressBar->maximum();

    if (m <= v)
    {
        ui->progressBar->setValue(m);
        ui->progressBar->setFormat(tr("传输完毕"));
        m_timer.stop();
    }
    else
    {
        ui->progressBar->setValue(++v);
        ui->progressBar->setFormat(tr("传输进度：%1%").arg(100 * v / m));
    }
}
