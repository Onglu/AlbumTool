#include "FindUserDialog.h"
#include "ui_FindUserDialog.h"

FindUserDialog::FindUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindUserDialog)
{
    ui->setupUi(this);
}

FindUserDialog::~FindUserDialog()
{
    delete ui;
}
