#ifndef ALBUMINFOWIDGET_H
#define ALBUMINFOWIDGET_H

#include <QWidget>

namespace Ui {
class AlbumInfoWidget;
}

class AlbumInfoWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit AlbumInfoWidget(QWidget *parent = 0);
    ~AlbumInfoWidget();

    void openWnd(const QStringList &businesses = QStringList());

    QString getBusinessName(void) const;

    bool isSampleAlbum(void) const;
    
signals:
    void hidden(bool ok);

private slots:
    void on_userRadioButton_clicked();

    void on_sampleRadioButton_clicked();

    void on_okPushButton_clicked(){emit hidden(true);}

    void on_cancelPushButton_clicked(){emit hidden(true);}

    void remove();

private:
    Ui::AlbumInfoWidget *ui;
};

#endif // ALBUMINFOWIDGET_H
