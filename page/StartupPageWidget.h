#ifndef STARTUPPAGEWIDGET_H
#define STARTUPPAGEWIDGET_H

#include <QWidget>
#include <QSettings>

class QPushButton;
class MainWindow;

class StartupPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StartupPageWidget(Qt::WindowFlags f, QSize fixed, QWidget *parent = 0);
    StartupPageWidget(QWidget *parent) : QWidget(parent){initTaskDir();}
    ~StartupPageWidget();

    void initTaskDir(void);

    enum OpenMethod{OpenMethod_New, OpenMethod_Open, OpenMethod_Save, OpenMethod_Saveas};
    const QString &getTaskFile(uchar mode, QString &taskFile, QString &taskName);

signals:
    
public slots:
    void openMw(){openMw("", "");}
    void newTask();
    void openTask();

private:
    void openMw(QString taskFile, QString taskName);
    void saveTask(const QString &taskFile);
    void parseTask(const QString &taskFile);

    QPushButton *m_pCloseBtn, *m_pNewBtn, *m_pOpenBtn;
    QSettings m_Settings;
    QString m_taskDir;
    static MainWindow *m_pMW;
};

#endif // STARTUPPAGEWIDGET_H
