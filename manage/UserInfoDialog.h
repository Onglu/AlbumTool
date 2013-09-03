#ifndef FINDUSERDIALOG_H
#define FINDUSERDIALOG_H

#include <QDialog>

namespace Ui {
class UserInfoDialog;
}

class UserInfoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UserInfoDialog(bool existing,
                            const QString &telephone,
                            const QString &realname = QString(),
                            uchar sex = 0,
                            QWidget *parent = 0);
    ~UserInfoDialog();

    // 获取用户名称
    QString getName(void) const {return m_name;}

    // 获取用户性别
    uchar getSex(void) const {return m_sex;}

    bool hasReset(void) const {return m_reset;}
    
private slots:
    void on_verifyPushButton_clicked();

    void on_createPushButton_clicked();

    void on_addPushButton2_clicked();

    void on_nameLineEdit_textChanged(const QString &arg1);

private:
    Ui::UserInfoDialog *ui;

    // 用户基本信息
    QString m_telno, m_name;
    uchar m_sex;
    bool m_reset;
};

#endif // FINDUSERDIALOG_H
