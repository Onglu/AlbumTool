#ifndef ALBUMMANAGEDIALOG_H
#define ALBUMMANAGEDIALOG_H

#include <QDialog>
#include <QTimer>

class QHttpRequestHeader;
class QHttp;

namespace Ui {
class AlbumManageDialog;
}

class AlbumManageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AlbumManageDialog(QWidget *parent = 0);
    ~AlbumManageDialog();
    
private slots:
    void on_operatePushButton_clicked();

    void httpSendProgress(int bytesSent, int bytesTotal);

    void httpRequestFinished(int, bool);

    void on_testPushButton_clicked();

    void end();

private:
    Ui::AlbumManageDialog *ui;
    QHttpRequestHeader *m_headr;
    QHttp *m_http;
    QTimer m_timer;
};

#endif // ALBUMMANAGEDIALOG_H
