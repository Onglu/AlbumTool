#ifndef FINDUSERDIALOG_H
#define FINDUSERDIALOG_H

#include <QDialog>

namespace Ui {
class FindUserDialog;
}

class FindUserDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit FindUserDialog(QWidget *parent = 0);
    ~FindUserDialog();
    
private:
    Ui::FindUserDialog *ui;
};

#endif // FINDUSERDIALOG_H
