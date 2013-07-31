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

    QString getName(void) const {return m_name;}

    uchar getSex(void) const {return m_sex;}

    bool hasReset(void) const {return m_reset;}
    
private slots:
    void on_verifyPushButton_clicked();

    void on_createPushButton_clicked();

    void on_addPushButton2_clicked();

    void on_nameLineEdit_textChanged(const QString &arg1);

private:
    Ui::UserInfoDialog *ui;
    QString m_telno, m_name;
    uchar m_sex;
    bool m_reset;
};

#endif // FINDUSERDIALOG_H
