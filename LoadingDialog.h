#ifndef LOADINGDLG_H
#define LOADINGDLG_H

#include <QDialog>

class QLabel;

class LoadingDialog : public QDialog
{
public:
    LoadingDialog(void);

    void showProcess(bool display,
                     QRect global = QRect(0, 0, 0, 0),
                     const QString &info = QString());

    const QString &getInfo(void) const;

private:
    QLabel *m_movieLabel, *m_textLabel;
};

class LoadingWidget : public QWidget
{
public:
    LoadingWidget(void);

    void showProcess(bool display,
                     QRect global = QRect(0, 0, 0, 0),
                     const QString &info = QString());

    const QString &getInfo(void) const;

private:
    QLabel *m_movieLabel, *m_textLabel;
};

#endif // LOADINGDLG_H
