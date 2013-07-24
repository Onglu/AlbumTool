#include "AlbumManageDialog.h"
#include "ui_AlbumManageDialog.h"
#include "AlbumTaskWidget.h"
#include "AlbumInfoWidget.h"
#include "LoadingDialog.h"
#include "parser/json.h"
#include "wrapper/utility.h"
#include <QDebug>
#include <QListWidget>
#include <QTextCodec>
#include <QMessageBox>

#define WIDGET_ITEM_HEIGHT      74
#define USER_LOGIN_URL          "http://192.168.2.120:8080/SwfUpload2/employeelogin?"
#define GET_BESINESS_URL        "http://192.168.2.120:8080/SwfUpload2/business/all/all_business.html"
#define USER_CREATE_URL         "http://192.168.2.120:8080/SwfUpload2/create"
#define CREATE_ALBUM_URL        "http://192.168.2.120:8080/SwfUpload2/createalbums?"
#define CREATE_SAMPLE_URL       "http://192.168.2.120:8080/SwfUpload2/createsample?"

using namespace QtJson;

class TaskWidgetItem : public QListWidgetItem
{
public:
    TaskWidgetItem(AlbumTaskWidget *widget, QListWidget *parent = 0) :
        QListWidgetItem(parent), m_widget(widget)
    {
        setSizeHint(QSize(0, WIDGET_ITEM_HEIGHT));
        if (parent)
        {
            parent->addItem(this);
            parent->setItemWidget(this, widget);
        }
    }

    ~TaskWidgetItem(void)
    {
        if (m_widget)
        {
            delete m_widget;
            m_widget = NULL;
        }
    }

    AlbumTaskWidget *getWidget(void) const {return m_widget;}

private:
    AlbumTaskWidget *m_widget;
};

AlbumManageDialog::AlbumManageDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint),
    ui(new Ui::AlbumManageDialog),
    m_loadingDlg(new LoadingDialog),
    m_logined(false),
    m_album(NULL)
{
    ui->setupUi(this);

    ui->nameLabel->hide();

    //ui->mainFrame->setEnabled(false);

    setMinimumSize(850, 600);

    connect(&m_watcher, SIGNAL(timeout()), SLOT(over()));

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

    m_info = new AlbumInfoWidget(this);
    ui->verticalLayout->addWidget(m_info);
    m_info->hide();
    connect(m_info, SIGNAL(hidden(bool)), SLOT(finish(bool)));
}

AlbumManageDialog::~AlbumManageDialog()
{
    while (!m_itemsList.isEmpty())
    {
        delete m_itemsList.takeFirst();
    }

    delete m_loadingDlg;

    delete ui;
}

void AlbumManageDialog::openWnd(const QStringList &albums)
{
    foreach (const QString &album, albums)
    {
        if (!album.isEmpty() && !m_albumsList.contains(album, Qt::CaseInsensitive))
        {
            m_albumsList.append(album);
            m_itemsList += new TaskWidgetItem(new AlbumTaskWidget(album, this), ui->listWidget);
        }
    }

    show();
}

void AlbumManageDialog::addTask(const QString &album)
{
    if (!album.isEmpty())
    {
        m_itemsList += new TaskWidgetItem(new AlbumTaskWidget(album, this), ui->listWidget);
    }
}

void AlbumManageDialog::switchPage()
{
    for (int i = 0; i < m_itemsList.size(); ++i)
    {
        TaskWidgetItem *taskItem = static_cast<TaskWidgetItem *>(m_itemsList.at(i));
        if (!taskItem)
        {
            continue;
        }

        if (ui->uploadPushButton->isChecked())
        {
            if (taskItem->isHidden())
            {
                taskItem->setHidden(false);
            }
        }
        else
        {
            AlbumTaskWidget::TaskState state = taskItem->getWidget()->getState();
            if (ui->uploadingPushButton->isChecked())
            {
                taskItem->setHidden((AlbumTaskWidget::Pause != state && AlbumTaskWidget::Start != state));
            }
            else if (ui->unuploadPushButton->isChecked())
            {
                taskItem->setHidden(AlbumTaskWidget::Initialize == state ? false : true);
            }
            else if (ui->uploadedPushButton->isChecked())
            {
                taskItem->setHidden((AlbumTaskWidget::Finished == state || AlbumTaskWidget::Published == state) ? false : true);
            }
        }
    }
}

void AlbumManageDialog::updateList()
{
    if (ui->uploadingPushButton->isChecked())
    {
        on_uploadingPushButton_clicked();
    }
    else if (ui->uploadingPushButton->isChecked())
    {
        on_unuploadPushButton_clicked();
    }
    else if (ui->uploadedPushButton->isChecked())
    {
        on_uploadedPushButton_clicked();
    }
}

void AlbumManageDialog::setInfo(AlbumTaskWidget &album)
{
    m_album = &album;

    ui->mainFrame->hide();

    QStringList businesses;
    foreach (const QVariant &businesse, m_businessesList)
    {
        QVariantMap values = businesse.toMap();
        QString name = values["name"].toString();
        if (!name.isEmpty())
        {
            businesses << name;
        }
    }

    //qDebug() << __FILE__ << __LINE__ << businesses;
    m_info->openWnd(businesses);
}

int AlbumManageDialog::getBusinessId(const QString &name) const
{
    foreach (const QVariant &businesse, m_businessesList)
    {
        QVariantMap values = businesse.toMap();
        if (name == values["name"].toString())
        {
            return values["id"].toInt();
        }
    }

    return 0;
}

void AlbumManageDialog::on_loginPushButton_clicked()
{
    m_uid = 0;

    if (!m_logined)
    {
        QString name = ui->nameLineEdit->text();
        QString pass = ui->passwdLineEdit->text();
        if (name.isEmpty() || pass.isEmpty())
        {
            QMessageBox::warning(this, tr("登陆失败"), tr("用户名及密码不能为空！"), tr("确定"));
            return;
        }

        m_url = tr("%1username=%2&password=%3").arg(USER_LOGIN_URL).arg(name).arg(pass);

        QUrl url(m_url);
        QNetworkRequest request(url);

        if (m_manager->get(request))
        {
            this->setEnabled(false);
            m_watcher.start(MAX_TIMEOUT);
            m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在登陆..."));
        }
    }
    else
    {
        m_logined = false;
        ui->nameLabel->hide();
        ui->nameLineEdit->clear();
        ui->nameLineEdit->show();
        ui->passwdLabel->show();
        ui->passwdLineEdit->clear();
        ui->passwdLineEdit->show();
        ui->loginPushButton->setText(tr("登陆"));
        ui->mainFrame->setEnabled(false);
    }
}

void AlbumManageDialog::over()
{
    m_watcher.stop();
    m_loadingDlg->showProcess(false);
    this->setEnabled(true);
    QMessageBox::information(this, tr("登陆超时"), tr("用户登陆超时，请稍后重试。"), tr("确定"));
}

void AlbumManageDialog::replyFinished(QNetworkReply *reply)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QByteArray ba = reply->readAll();
    QVariantMap result = QtJson::parse(QString(ba)).toMap();
    int code = result["protocol"].toInt();

    //qDebug() << __FILE__ << __LINE__ << ba << m_url;
    reply->deleteLater();

    if (m_url.startsWith(USER_LOGIN_URL))
    {
        if (!m_logined && !m_watcher.isActive())
        {
            goto end;
        }

        m_watcher.stop();
        m_loadingDlg->showProcess(false);
        this->setEnabled(true);

        if (SERVER_REPLY_SUCCESS != code)
        {
            QMessageBox::information(this, tr("认证失败"), tr("登陆认证失败！错误码：%1").arg(code), tr("确定"));
            goto end;
        }

        QVariantMap value = result["retValue"].toMap();
        if (!value.isEmpty())
        {
            m_uid = value["id"].toInt();
            m_userKey = value["userkey"].toString();
            if (!m_userKey.isEmpty())
            {
                //qDebug() << __FILE__ << __LINE__ << value["id"].toInt() << value["userkey"].toString() << ui->nameLineEdit->text();
                m_logined = true;
                ui->nameLabel->show();
                ui->nameLabel->setText(ui->nameLineEdit->text());
                ui->nameLineEdit->hide();
                ui->passwdLabel->hide();
                ui->passwdLineEdit->hide();
                ui->loginPushButton->setText(tr("退出"));
                ui->mainFrame->setEnabled(true);

                m_url = GET_BESINESS_URL;
                QNetworkRequest request(QUrl(GET_BESINESS_URL));
                if (m_manager->get(request))
                {
                    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
                    return;
                }
            }
        }
    }
    else
    {
        if (SERVER_REPLY_SUCCESS != code)
        {
            QMessageBox::information(this, tr("操作失败"), tr("错误码：%1").arg(code), tr("确定"));
            goto end;
        }

        if (m_url.startsWith(GET_BESINESS_URL))
        {
            m_businessesList = result["retValue"].toList();
            //qDebug() << __FILE__ << __LINE__ << m_businessesList;
        }

        if (m_url.startsWith(USER_CREATE_URL))
        {
            QVariantMap value = result["retValue"].toMap();
            if (!value.isEmpty() && m_album)
            {
                m_album->start(value["id"].toInt(), m_url.startsWith(CREATE_ALBUM_URL) ? USER_ALBUM : SAMPLE_ALBUM);
                m_album = NULL;
            }
        }
    }

end:
    m_url.clear();

    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
}

void AlbumManageDialog::createAlbum(int bid, bool sample)
{
    if (!m_album)
    {
        return;
    }

    if (sample)
    {
        m_url = tr("%1userkey=%2&businessid=%3&name=%4&summary=none&fileguid=%5&md5=%6&employeeid=%7").arg(CREATE_SAMPLE_URL).arg(m_userKey).arg(bid).arg(m_album->getName()).arg(m_album->getUuid()).arg(m_album->getMd5()).arg(m_uid);
    }
    else
    {
        m_url = tr("%1userkey=%2&businessid=%3&name=%4&summary=none&fileguid=%5&md5=%6&size=%7&pagenum=%8&photonum=%9&cusids=%10&employeeid=%11").arg(CREATE_ALBUM_URL).arg(m_userKey).arg(bid).arg(m_album->getName()).arg(m_album->getUuid()).arg(m_album->getMd5()).arg(m_album->getSize()).arg(m_album->getPagesNum()).arg(m_album->getPhotosNum()).arg("1,2").arg(m_uid);
    }

    qDebug() << __FILE__ << __LINE__ << m_url;

    QUrl url(m_url);
    QNetworkRequest request(url);
    m_manager->get(request);
}

void AlbumManageDialog::finish(bool ok)
{
    if (ok && m_album)
    {
        createAlbum(getBusinessId(m_info->getBusinessName()), m_info->isSampleAlbum());
    }

    m_info->hide();
    ui->mainFrame->show();
}

void AlbumManageDialog::closeEvent(QCloseEvent *)
{
    if (m_info->isVisible())
    {
        m_info->hide();
        ui->mainFrame->show();
    }
}
