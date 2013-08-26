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
#include <QCryptographicHash>

#define WIDGET_ITEM_HEIGHT      74

#ifdef PUBLIC_ENV
#define USER_LOGIN_URL          "http://www.ccg365.com:9080/SwfUpload2/employeelogin?"
#define GET_BESINESS_URL        "http://www.ccg365.com:9080/SwfUpload2/business/all/all_business.html"
#define CREATE_ALBUM_URL        "http://www.ccg365.com:9080/SwfUpload2/createalbums?"
#define CREATE_SAMPLE_URL       "http://www.ccg365.com:9080/SwfUpload2/createsample?"
#define UPDATE_ALBUMSAMPLE_URL  "http://www.ccg365.com:9080/SwfUpload2/updatealbumsample?"
#define GET_ALBUMSAMPLE_URL     "http://www.ccg365.com:9080/SwfUpload2/getalbumsampleinfo?"
#define FIND_USER_URL           "http://www.ccg365.com:9080/SwfUpload2/finduser?"
#define CREATE_USER_URL         "http://www.ccg365.com:9080/SwfUpload2/createuser?"
#else
#define USER_LOGIN_URL          "http://192.168.2.120:8080/SwfUpload2/employeelogin?"
#define GET_BESINESS_URL        "http://192.168.2.120:8080/SwfUpload2/business/all/all_business.html"
#define CREATE_ALBUM_URL        "http://192.168.2.120:8080/SwfUpload2/createalbums?"
#define CREATE_SAMPLE_URL       "http://192.168.2.120:8080/SwfUpload2/createsample?"
#define UPDATE_ALBUMSAMPLE_URL  "http://192.168.2.120:8080/SwfUpload2/updatealbumsample?"
#define GET_ALBUMSAMPLE_URL     "http://192.168.2.120:8080/SwfUpload2/getalbumsampleinfo?"
#define FIND_USER_URL           "http://192.168.2.120:8080/SwfUpload2/finduser?"
#define CREATE_USER_URL         "http://192.168.2.120:8080/SwfUpload2/createuser?"
#endif

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
    m_currTask(NULL)
{
    ui->setupUi(this);

    ui->nameLabel->hide();

    ui->mainFrame->setEnabled(false);

    setMinimumSize(850, 600);

    connect(&m_watcher, SIGNAL(timeout()), SLOT(over()));

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

    m_setPage = new AlbumInfoWidget(false, this);
    ui->verticalLayout->addWidget(m_setPage);
    m_setPage->hide();

    m_editPage = new AlbumInfoWidget(true, this);
    ui->verticalLayout->addWidget(m_editPage);
    m_editPage->hide();

    connect(m_setPage, SIGNAL(accepted(bool,QString,bool)), SLOT(accept(bool,QString,bool)));
    connect(m_editPage, SIGNAL(accepted(bool,QString,bool)), SLOT(accept(bool,QString,bool)));
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

    if (isHidden())
    {
        show();
    }
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

void AlbumManageDialog::setAlbumInfo(AlbumTaskWidget &task, bool view)
{
    if (m_currTask != &task)
    {
        m_currTask = &task;
    }

    ui->mainFrame->hide();

    if (!view)
    {
        QStringList businesses;
        m_setPage->openWnd(getBusinesses(businesses));
    }
    else
    {
        int aid = m_currTask->getAlbumId();
        if (aid)
        {
            m_url = QString("%1employeeid=%2&userkey=%3&type=%4&id=%5").arg(GET_ALBUMSAMPLE_URL).arg(m_uid).arg(m_userKey).arg(m_currTask->getAlbumType()).arg(aid);
            QUrl url(m_url);
            QNetworkRequest request(url);
            m_manager->get(request);
        }
        else
        {
            m_editPage->openWnd();
        }
    }
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

QString AlbumManageDialog::getBusinessName(int id, QString &name) const
{
    foreach (const QVariant &businesse, m_businessesList)
    {
        QVariantMap values = businesse.toMap();
        if (id == values["id"].toInt())
        {
            name = values["name"].toString();
            break;
        }
    }

    return name;
}

const QStringList &AlbumManageDialog::getBusinesses(QStringList &businesses) const
{
    foreach (const QVariant &businesse, m_businessesList)
    {
        QVariantMap values = businesse.toMap();
        QString name = values["name"].toString();
        if (!name.isEmpty())
        {
            businesses << name;
        }
    }

    return businesses;
}

void AlbumManageDialog::on_loginPushButton_clicked()
{
    m_uid = 0;

    if (!m_logined)
    {
        QString name = ui->nameLineEdit->text();
        QString pwd = ui->passwdLineEdit->text();
        if (name.isEmpty() || pwd.isEmpty())
        {
            QMessageBox::warning(this, tr("登录失败"), tr("登录名或密码不正确！"), tr("确定"));
            return;
        }

        QString md5;
        QByteArray bb;
        bb = QCryptographicHash::hash(pwd.toAscii(), QCryptographicHash::Md5);
        md5.append(bb.toHex());

        m_url = tr("%1username=%2&password=%3").arg(USER_LOGIN_URL).arg(name).arg(md5);
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

void AlbumManageDialog::findUser(const QString &telephone)
{
    m_url = QString("%1telephone=%2&employeeid=%3&userkey=%4").arg(FIND_USER_URL).arg(telephone).arg(m_uid).arg(m_userKey);
    QUrl url(m_url);
    QNetworkRequest request(url);
    if (m_manager->get(request))
    {
        m_watcher.start(MAX_TIMEOUT);
        m_loadingDlg->showProcess(true, QRect(this->mapToGlobal(QPoint(0, 0)), this->size()), tr("正在查找..."));
    }
}

void AlbumManageDialog::addUser(const QString &telephone,
                                const QString &realname,
                                uchar sex)
{
    m_url = tr("%1telephone=%2&realname=%3&male=%4&employeeid=%5&userkey=%6").arg(CREATE_USER_URL).arg(telephone).arg(realname).arg(sex).arg(m_uid).arg(m_userKey);
    QUrl url(m_url);
    QNetworkRequest request(url);
    m_manager->get(request);
}

void AlbumManageDialog::over()
{
    m_watcher.stop();
    m_loadingDlg->showProcess(false);

    if (m_url.startsWith(USER_LOGIN_URL))
    {
        this->setEnabled(true);
        QMessageBox::information(this, tr("登录超时"), tr("用户登录超时，请稍后重试。"), tr("确定"));
    }
    else if (m_url.startsWith(FIND_USER_URL))
    {
        QMessageBox::information(this, tr("查找超时"), tr("查找操作已超时，请稍后重试。"), tr("确定"));
    }
}

void AlbumManageDialog::replyFinished(QNetworkReply *reply)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QByteArray ba = reply->readAll();
    QVariantMap result = QtJson::parse(QString(ba)).toMap();
    int code = result["protocol"].toInt();

    if (m_url.isEmpty())
    {
        m_url = reply->url().toString();
    }

    qDebug() << __FILE__ << __LINE__ << m_url;
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
            QMessageBox::information(this, tr("认证失败"), tr("登录认证失败！错误码：%1").arg(code), tr("确定"));
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
//        if (SERVER_REPLY_SUCCESS != code)
//        {
//            QMessageBox::information(this, tr("操作失败"), tr("错误码：%1").arg(code), tr("确定"));
//            goto end;
//        }

        //qDebug() << __FILE__ << __LINE__ << m_url /*<< result*/;

        if (m_url.startsWith(GET_BESINESS_URL) && SERVER_REPLY_SUCCESS == code)
        {
            m_businessesList = result["retValue"].toList();
            //qDebug() << __FILE__ << __LINE__ << m_businessesList;
        }

        if (m_currTask)
        {
            QVariantMap value = result["retValue"].toMap();

            if (m_url.startsWith(GET_ALBUMSAMPLE_URL))
            {
                if (SERVER_REPLY_SUCCESS == code)
                {
                    m_currTask->setAlbumInfo(value);
                }

                m_editPage->openWnd();
            }

            if (m_url.startsWith(CREATE_ALBUM_URL) || m_url.startsWith(CREATE_SAMPLE_URL) || m_url.startsWith(UPDATE_ALBUMSAMPLE_URL))
            {
                if (SERVER_REPLY_SUCCESS != code)
                {
                    QMessageBox::information(this, tr("操作失败"), tr("创建相册失败，错误码：%1").arg(code), tr("确定"));
                }
                else
                {
                    if (!value.isEmpty())
                    {
                        m_currTask->start(value["id"].toInt());
                    }
                }
            }

            if (m_url.startsWith(FIND_USER_URL))
            {
                m_watcher.stop();
                m_loadingDlg->showProcess(false);
                m_setPage->bindUser(value, code);
            }

            if (m_url.startsWith(CREATE_USER_URL) && SERVER_REPLY_SUCCESS == code)
            {
                m_setPage->bindUser(value);
            }
        }
    }

end:
    m_url.clear();

    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
}

void AlbumManageDialog::accept(bool ok, const QString &business, bool sample)
{
    int bid = 0;
    bool editing = m_editPage->isEditing();

    if (ok && m_currTask && (bid = getBusinessId(business)))
    {
        QString ids(QChar(0));
        quint64 size = 0;
        int pagesNum = 0;
        int photosNum = 0;

        if (!sample)
        {
            if (m_currTask->getUsersId(ids).isEmpty())
            {
                QMessageBox::information(this, tr("操作失败"), tr("关联用户列表失败，请重新进行设置！"), tr("确定"));
                return;
            }
            else
            {
                size = m_currTask->getSize();
                pagesNum = m_currTask->getPagesNum();
                photosNum = m_currTask->getPhotosNum();
            }
        }

        QString name = m_currTask->getName();
        QString uuid = m_currTask->getUuid();
        QString md5 = m_currTask->getMd5();
        uchar type = sample ? SAMPLE_ALBUM : USER_ALBUM;

        if (editing)
        {
            m_url = tr("%1userkey=%2&businessid=%3&name=%4&summary=none&fileguid=%5&md5=%6&size=%7&pagenum=%8&photonum=%9&cusids=%10&employeeid=%11&oldtype=%12&newtype=%13&id=%14").arg(UPDATE_ALBUMSAMPLE_URL).arg(m_userKey).arg(bid).arg(name).arg(uuid).arg(md5).arg(size).arg(pagesNum).arg(photosNum).arg(ids).arg(m_uid).arg(m_currTask->getAlbumType()).arg(type).arg(m_currTask->getAlbumId());
        }
        else
        {
            if (sample)
            {
                m_url = tr("%1userkey=%2&businessid=%3&name=%4&summary=none&fileguid=%5&md5=%6&employeeid=%7").arg(CREATE_SAMPLE_URL).arg(m_userKey).arg(bid).arg(name).arg(uuid).arg(md5).arg(m_uid);
            }
            else
            {
                m_url = tr("%1userkey=%2&businessid=%3&name=%4&summary=none&fileguid=%5&md5=%6&size=%7&pagenum=%8&photonum=%9&cusids=%10&employeeid=%11").arg(CREATE_ALBUM_URL).arg(m_userKey).arg(bid).arg(name).arg(uuid).arg(md5).arg(size).arg(pagesNum).arg(photosNum).arg(ids).arg(m_uid);
            }
        }

        //qDebug() << __FILE__ << __LINE__ << editing << m_url;

        QUrl url(m_url);
        QNetworkRequest request(url);
        if (m_manager->get(request))
        {
            m_currTask->setRelevance(type, business);
        }
    }

    if (m_editPage->isVisible())
    {
        m_editPage->hide();
    }

    if (m_setPage->isVisible())
    {
        m_setPage->hide();
        if (editing)
        {
            m_editPage->openWnd();
            return;
        }
        else
        {
            if (!ok && m_currTask)
            {
                m_currTask = NULL;
            }
        }
    }

    ui->mainFrame->show();
}

void AlbumManageDialog::closeEvent(QCloseEvent *)
{
    if (m_editPage->isVisible())
    {
        m_editPage->hide();
        ui->mainFrame->show();
    }

    if (m_setPage->isVisible())
    {
        m_setPage->hide();
        ui->mainFrame->show();
    }
}

void AlbumManageDialog::on_keywordLineEdit_textChanged(const QString &arg1)
{
    int i = 0;
    bool v = arg1.isEmpty();
    ui->searchPushButton->setEnabled(!v);

    do
    {
        TaskWidgetItem *taskItem = static_cast<TaskWidgetItem *>(m_itemsList.at(i));
        if (taskItem)
        {
            taskItem->setHidden(false);
        }
    } while (v && ++i < m_itemsList.size());
}

void AlbumManageDialog::on_searchPushButton_clicked()
{
    QString keyword = ui->keywordLineEdit->text();

    for (int i = 0; i < m_itemsList.size(); ++i)
    {
        TaskWidgetItem *taskItem = static_cast<TaskWidgetItem *>(m_itemsList.at(i));
        if (taskItem)
        {
            QString name = taskItem->getWidget()->getName();
            bool v = name.contains(keyword, Qt::CaseInsensitive) || keyword.contains(name, Qt::CaseInsensitive);
            taskItem->setHidden(!v);
        }
    }
}
