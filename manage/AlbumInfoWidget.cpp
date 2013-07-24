#include "AlbumInfoWidget.h"
#include "ui_AlbumInfoWidget.h"
#include <QDebug>

AlbumInfoWidget::AlbumInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlbumInfoWidget)
{
    ui->setupUi(this);

//    QPushButton *button = new QPushButton(tr("移除"));
//    button->setFixedSize(75, 32);
//    connect(button, SIGNAL(clicked()), SLOT(remove()));

//    QListWidgetItem *item = new QListWidgetItem(tr("手机号码：123 1234 1234"), ui->listWidget);
//    item->setSizeHint(QSize(0, 32));
//    ui->listWidget->setItemWidget(item, button);

//    item = new QListWidgetItem(tr("手机号码：123 1111 1111"), ui->listWidget);
//    item->setSizeHint(QSize(0, 32));
//    ui->listWidget->setItemWidget(item, button);

    for (int i = 0; i < 3; i++)
    {
        QLabel *label = new QLabel(tr("手机号码：%1").arg(i));

        QPushButton *button = new QPushButton(tr("移除"));
        button->setFixedSize(75, 23);
        connect(button, SIGNAL(clicked()), SLOT(remove()));

        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget(label);
        layout->addWidget(button);

        QWidget *widget = new QWidget();
        widget->setLayout(layout);

        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(widget->sizeHint());
        ui->listWidget->setItemWidget(item, widget);
    }
}

AlbumInfoWidget::~AlbumInfoWidget()
{
    delete ui;
}

void AlbumInfoWidget::remove()
{
    qDebug() << __FILE__ << __LINE__ << ui->listWidget->currentRow();
}

void AlbumInfoWidget::openWnd(const QStringList &businesses)
{
    ui->comboBox->addItems(businesses);
    show();
}

void AlbumInfoWidget::on_userRadioButton_clicked()
{
    ui->usersFrame->show();
}

void AlbumInfoWidget::on_sampleRadioButton_clicked()
{
    ui->usersFrame->hide();
}

QString AlbumInfoWidget::getBusinessName() const
{
    return ui->comboBox->currentText();
}

bool AlbumInfoWidget::isSampleAlbum() const
{
    return ui->sampleRadioButton->isChecked();
}
