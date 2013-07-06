#ifndef LOADINGDLG_H
#define LOADINGDLG_H

#include <QDialog>

class QLabel;

class LoadingDlg : public QDialog
{
    Q_OBJECT

public:
    LoadingDlg(void);

    void showProcess(bool show,
                     QRect global = QRect(0, 0, 0, 0),
                     const QString &info = QString());

signals:
    void ss(bool);

protected:
    void showEvent(QShowEvent *);

private:
    QLabel *m_movieLabel, *m_textLabel;
};

#endif // LOADINGDLG_H
