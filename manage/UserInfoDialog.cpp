#include "UserInfoDialog.h"
#include "ui_UserInfoDialog.h"

UserInfoDialog::UserInfoDialog(bool existing,
                               const QString &telephone,
                               const QString &realname,
                               uchar sex,
                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserInfoDialog),
    m_name(realname),
    m_sex(0),
    m_reset(false)
{
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    ui->verifyFrame->hide();
    ui->telnoLabel1->setText(telephone);
    ui->telnoLabel2->setText(telephone);
    ui->telnoLabel3->setText(telephone);

    if (existing)
    {
        ui->createFrame->hide();
        ui->nameLabel->setText(realname);

        if (2 < sex)
        {
            sex = 0;
        }

        const char *name[] = {"未知", "男", "女"};
        ui->sexLabel->setText(tr("%1").arg(name[sex]));
    }
    else
    {
        ui->confirmFrame->hide();
        ui->nameLineEdit->setFocus();
    }

    setFixedSize(406, 300);
}

UserInfoDialog::~UserInfoDialog()
{
    delete ui;
}

void UserInfoDialog::on_verifyPushButton_clicked()
{
    ui->confirmFrame->hide();
    ui->verifyFrame->show();
}

void UserInfoDialog::on_createPushButton_clicked()
{
    m_reset = true;
    ui->verifyFrame->hide();
    ui->createFrame->show();
    ui->nameLineEdit->setFocus();
}

void UserInfoDialog::on_addPushButton2_clicked()
{
    m_name = ui->nameLineEdit->text();

    if (ui->maleRadioButton->isChecked())
    {
        m_sex = 1;
    }
    else if (ui->femaleRadioButton->isChecked())
    {
        m_sex = 2;
    }
    else
    {
        m_sex = 0;
    }

    accept();
}

void UserInfoDialog::on_nameLineEdit_textChanged(const QString &arg1)
{
    ui->addPushButton2->setEnabled(!arg1.isEmpty());
}
