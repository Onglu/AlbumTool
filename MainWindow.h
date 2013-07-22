#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSettings>

class AlbumManageDialog;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &taskFile, const QString &taskName, QWidget *parent = 0);
    ~MainWindow();

    bool hasOpened(const QString &taskFile);

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_tabWidget_tabCloseRequested(int index);

    void on_newAction_triggered();

    void on_openAction_triggered();

    void on_saveAction_triggered();

    void on_saveasAction_triggered();

    void on_importTemplatesAction_triggered();

    void onChanged(int index);

    void changeTab(int index);

    void showMax(bool);

    void on_manageTemplateAction_triggered();

    void on_manageAlbumAction_triggered();

private:
    bool closePage(int index);

    enum AddMethod{AddMethod_Default/* Only add one page */, AddMethod_New, AddMethod_Open};
    void addPage(AddMethod method, QString taskFile = "", QString taskName = "");

    Ui::MainWindow *ui;
    QLabel *m_pLabel, *m_pSsLabel;
    QSettings m_Settings;
    AlbumManageDialog *m_albums;
};

#endif // MAINWINDOW_H
