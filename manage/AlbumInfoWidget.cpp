#include "AlbumInfoWidget.h"
#include "ui_AlbumInfoWidget.h"
#include "AlbumManageDialog.h"
#include "AlbumTaskWidget.h"
#include "UserInfoDialog.h"
#include <QDebug>
#include <QMessageBox>

UserInfoWidget::UserInfoWidget(int index, int id, const QString &info, AlbumInfoWidget &container) :
    QWidget(0),
    m_index(index),
    m_id(id),
    m_info(info),
    m_container(container)
{
    QLabel *label = new QLabel(info);

    QPushButton *button = new QPushButton(tr("移除"));
    button->setFixedSize(75, 23);
    connect(button, SIGNAL(clicked()), SLOT(remove()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(label);
    layout->addWidget(button);

    setLayout(layout);
}

AlbumInfoWidget::AlbumInfoWidget(bool editable, AlbumManageDialog *parent) :
    QWidget(parent),
    ui(new Ui::AlbumInfoWidget),
    m_container(parent)
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    if (editable)
    {
        ui->nameLineEdit->hide();
        ui->cancelPushButton2->hide();
        ui->okPushButton->hide();
        ui->setFrame->hide();
    }
    else
    {
        ui->editFrame->hide();
    }
}

AlbumInfoWidget::~AlbumInfoWidget()
{
    delete ui;
}

void AlbumInfoWidget::openWnd(const QStringList &businesses)
{
    AlbumTaskWidget *task = m_container->getCurrTask();
    if (!task)
    {
        return;
    }

    if (isHidden())
    {
        show();
    }

    int index = 0;
    uchar atype;
    QString business;

    task->getRelevance(atype, business);

    ui->comboBox->clear();
    ui->comboBox->addItems(businesses);

    foreach (const QString &name, businesses)
    {
        if (business == name)
        {
            ui->comboBox->setCurrentIndex(index);
            break;
        }

        index++;
    }

    if (USER_ALBUM == atype)
    {
        ui->userRadioButton->setChecked(true);
        ui->usersListWidgets1->clear();
        showUsers();

        UserInfoItems items = task->getUsers();
        UserInfoItems::const_iterator iter = items.constBegin();
        while (iter != items.constEnd())
        {
            UserInfoWidget *widget = new UserInfoWidget(ui->usersListWidgets1->count(), iter.key(), iter.value(), *this);
            QListWidgetItem *item = new QListWidgetItem(ui->usersListWidgets1);
            item->setSizeHint(widget->sizeHint());
            ui->usersListWidgets1->setItemWidget(item, widget);
            ++iter;
        }
    }
    else
    {
        ui->sampleRadioButton->setChecked(true);
        showUsers(false);
    }

    ui->telnoLineEdit->clear();
}

void AlbumInfoWidget::openWnd()
{
    AlbumTaskWidget *task = m_container->getCurrTask();
    if (!task)
    {
        return;
    }

    if (isHidden())
    {
        show();
    }

    ui->nameLabel->setText(task->getName());
    ui->numLabel->setText(tr("本相册共 %1 页，入册照片 %2 张，剩余空位 %3 个。").arg(task->getPagesNum()).arg(task->getPhotosNum()).arg(task->getBlankNum()));

    uchar atype;
    QString business;

    task->getRelevance(atype, business);
    ui->typeLabel->setText(tr("相册类型：%1").arg(USER_ALBUM == atype ? tr("这是一本为用户制作的相册") : tr("这是一本影楼自己的样册")));
    ui->businessLabel->setText(tr("相关影楼：%1").arg(business));

    if (USER_ALBUM == atype)
    {
        showUsers();
        ui->usersListWidgets2->clear();
        //qDebug() << __FILE__ << __LINE__ << task << users;

        UserInfoItems items = task->getUsers();
        UserInfoItems::const_iterator iter = items.constBegin();
        while (iter != items.constEnd())
        {
            QListWidgetItem *item = new QListWidgetItem(iter.value(), ui->usersListWidgets2);
            item->setSizeHint(QSize(0, 41));
            ++iter;
        }
    }
    else
    {
        showUsers(false);
    }
}

void AlbumInfoWidget::bindUser(const QVariantMap &user, int code)
{
    //qDebug() << __FILE__ << __LINE__ << user;

    if (INVALID_TELNO == code)
    {
        QMessageBox::information(this, tr("查找失败"), tr("输入的电话号码格式有误，请重新输入！"), tr("确定"));
        ui->telnoLineEdit->setFocus();
    }
    else if (SERVER_REPLY_SUCCESS == code)
    {
        UserInfoDialog dlg(true, user["telephone"].toString(), user["realname"].toString(), (uchar)user["male"].toUInt());
        if (QDialog::Accepted == dlg.exec())
        {
            if (dlg.hasReset())
            {
                m_container->addUser(ui->telnoLineEdit->text(), dlg.getName(), dlg.getSex());
            }
            else
            {
                addUser(user);
            }
        }
    }
    else if (NO_RESULT == code)
    {
        UserInfoDialog dlg(false, ui->telnoLineEdit->text());
        if (QDialog::Accepted == dlg.exec())
        {
            m_container->addUser(ui->telnoLineEdit->text(), dlg.getName(), dlg.getSex());
        }
    }
}

void AlbumInfoWidget::addUser(const QVariantMap &user)
{
    AlbumTaskWidget *task = m_container->getCurrTask();
    if (!task || user.isEmpty())
    {
        return;
    }

    //qDebug() << __FILE__ << __LINE__ << user;

    UserInfoWidget *widget = NULL;
    QListWidgetItem *item = NULL;
    int num = ui->usersListWidgets1->count();
    int uid = user["id"].toInt();
    QString base = tr(" 手机号码：%1").arg(user["telephone"].toString());
    QString info = tr("%1\t\t姓名：%2\t\t初始密码：%3").arg(base).arg(user["realname"].toString()).arg(user["initpassword"].toString());

    for (int i = 0; i < num; i++)
    {
        if ((item = ui->usersListWidgets1->item(i)) &&
            (widget = static_cast<UserInfoWidget *>(ui->usersListWidgets1->itemWidget(item))))
        {
            QString data = widget->getInfo();
            if (info == data)
            {
                item->setSelected(true);
                return;
            }
            else if (data.startsWith(base))
            {
                removeUser(i);
                break;
            }
        }
    }

    widget = new UserInfoWidget(ui->usersListWidgets1->count(), uid, info, *this);
    item = new QListWidgetItem(ui->usersListWidgets1);
    item->setSizeHint(widget->sizeHint());
    ui->usersListWidgets1->setItemWidget(item, widget);
}

void AlbumInfoWidget::removeUser(int row)
{
    QListWidgetItem *item = ui->usersListWidgets1->item(row);
    if (!item)
    {
        return;
    }

    UserInfoWidget *widget = static_cast<UserInfoWidget *>(ui->usersListWidgets1->itemWidget(item));
    if (widget)
    {
        delete widget;
    }

    delete item;

    int num = ui->usersListWidgets1->count();
    for (int i = row; i < num; i++)
    {
        item = ui->usersListWidgets1->item(i);
        if ((widget = static_cast<UserInfoWidget *>(ui->usersListWidgets1->itemWidget(item))))
        {
            widget->setIndex(i);
        }
    }
}

void AlbumInfoWidget::showUsers(bool visiable)
{
    if (ui->setFrame->isVisible())
    {
        ui->setLabel->setVisible(visiable);
        ui->phoneLabel->setVisible(visiable);
        ui->telnoLineEdit->setVisible(visiable);
        ui->findPushButton->setVisible(visiable);
        ui->usersLabel1->setVisible(visiable);
        ui->usersListWidgets1->setVisible(visiable);
    }

    if (ui->editFrame->isVisible())
    {
        ui->usersLabel2->setVisible(visiable);
        ui->usersListWidgets2->setVisible(visiable);
    }
}

QString AlbumInfoWidget::getUsersId(QString &ids)
{
    if (ui->setFrame->isVisible() && ui->userRadioButton->isChecked())
    {
        UserInfoWidget *widget = NULL;
        QListWidgetItem *item = NULL;
        int num = ui->usersListWidgets1->count();

        for (int i = 0; i < num; i++)
        {
            if ((item = ui->usersListWidgets1->item(i))
                    && (widget = static_cast<UserInfoWidget *>(ui->usersListWidgets1->itemWidget(item))))
            {
                if (!i)
                {
                    ids = QString("%1").arg(widget->getId());
                }
                else
                {
                    ids.append(QString(",%1").arg(widget->getId()));
                }
            }
        }
    }

    return ids;
}

QString AlbumInfoWidget::getBusinessName() const
{
    return ui->comboBox->currentText();
}

bool AlbumInfoWidget::isSampleAlbum() const
{
    return ui->sampleRadioButton->isChecked();
}

void AlbumInfoWidget::on_telnoLineEdit_textChanged(const QString &arg1)
{
    ui->findPushButton->setEnabled(!arg1.isEmpty());
}

void AlbumInfoWidget::on_findPushButton_clicked()
{
    m_container->findUser(ui->telnoLineEdit->text());
}

void AlbumInfoWidget::on_okPushButton_clicked()
{
    AlbumTaskWidget *task = m_container->getCurrTask();
    if (!task)
    {
        return;
    }

    UserInfoItems users;
    bool sample = ui->sampleRadioButton->isChecked();

    if (sample)
    {
        ui->usersListWidgets1->clear();
    }
    else
    {
        int num = ui->usersListWidgets1->count();
        if (!num)
        {
            QMessageBox::information(this, tr("创建失败"), tr("必须要添加一个关联用户才能创建此种类型的相册！"), tr("确定"));
            return;
        }

        UserInfoWidget *widget = NULL;
        QListWidgetItem *item = NULL;
        for (int i = 0; i < num; i++)
        {
            if ((item = ui->usersListWidgets1->item(i))
                && (widget = static_cast<UserInfoWidget *>(ui->usersListWidgets1->itemWidget(item))))
            {
                users[widget->getId()] = widget->getInfo();
            }
        }
    }

    task->setUsers(users);
    emit accepted(true, ui->comboBox->currentText(), sample);
}

void AlbumInfoWidget::on_renamePushButton_clicked()
{
    if (ui->nameLineEdit->isHidden())
    {
        m_name = ui->nameLabel->text();
        ui->nameLabel->hide();
        ui->nameLineEdit->setText(m_name);
        ui->nameLineEdit->show();
        ui->nameLineEdit->setFocus();
        ui->cancelPushButton2->show();
        ui->renamePushButton->setText(tr("确定"));
    }
    else
    {
        m_name = ui->nameLineEdit->text();
        if (m_name.isEmpty())
        {
            QMessageBox::information(this, tr("名称无效"), tr("相册名称不能为空！"), tr("确定"));
            ui->nameLineEdit->setFocus();
            return;
        }

        AlbumTaskWidget *task = m_container->getCurrTask();
        if (task)
        {
            task->setName(m_name);
        }

        ui->nameLabel->setText(m_name);
        ui->nameLabel->show();
        ui->nameLineEdit->hide();
        ui->renamePushButton->setText(tr("修改名称"));
        ui->cancelPushButton2->hide();
    }
}

void AlbumInfoWidget::on_cancelPushButton2_clicked()
{
    ui->nameLabel->setText(m_name);
    ui->nameLabel->show();
    ui->nameLineEdit->hide();
    ui->renamePushButton->setText(tr("修改名称"));
    ui->cancelPushButton2->hide();
}

void AlbumInfoWidget::on_setPushButton_clicked()
{
    m_editing = true;
    hide();
    m_container->setAlbumInfo(*m_container->getCurrTask());
}

void AlbumInfoWidget::on_previewPushButton_clicked()
{
    AlbumTaskWidget *task = m_container->getCurrTask();
    if (task)
    {
        task->onPreview();
    }
}
