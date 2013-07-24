#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PreviewDialog.h"
#include "manage/AlbumManageDialog.h"
#include "page/StartupPageWidget.h"
#include "page/TaskPageWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>

extern QRect g_appRect;

MainWindow::MainWindow(const QString &taskFile, const QString &taskName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_albums(NULL)
{
    ui->setupUi(this);

    //ui->menuBar->hide();

    m_albums = new AlbumManageDialog;

//    m_pLabel = new QLabel(tr("准备"));    // Ready
//    //m_pLabel->setMinimumSize(m_pLabel->sizeHint());
//    ui->statusBar->addWidget(m_pLabel);

//    m_pSsLabel = new QLabel(tr("Setting"));
//    ui->statusBar->addWidget(m_pSsLabel, 1);

    ui->statusBar->hide();

    /* Support for the minimumsize resolution of 10.1 inches notebook */
    setMinimumSize(MINI_WIDTH, MINI_HEIGHT);

    m_Settings.beginGroup("MainWindow");
    QRect rect = m_Settings.value("geometry").toRect();
    if (QPoint(0, 0) != rect.topLeft())
    {
        setGeometry(rect);
    }
    m_Settings.endGroup();

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(changeTab(int)));

//    QFont font(ui->importLabel->font());
//    QFontMetrics fm(font);
//    ui->importLabel->setText(fm.elidedText(ui->importLabel->text(), Qt::ElideRight, ui->importLabel->width()));
//    ui->importLabel->adjustSize();
//    ui->importLabel->setStyleSheet("color:blue;");

    addPage(AddMethod_Default, taskFile, taskName);
}

MainWindow::~MainWindow()
{
    delete m_albums;
    delete ui;
}

void MainWindow::showMax(bool max)
{
    static QRect rect;

    if (max)
    {
        int top = ui->menuBar->isVisible() ? ui->menuBar->height() : 0;
        rect = geometry();
        setFixedSize(g_appRect.width(), g_appRect.height() - ui->menuBar->height());
        setGeometry(0, top, g_appRect.width(), g_appRect.height() - top);
        //qDebug() << __FILE__ << __LINE__ << rect << g_appRect;
    }
    else
    {
        setMinimumSize(MINI_WIDTH, MINI_HEIGHT);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setGeometry(rect);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_Settings.beginGroup("MainWindow");
    m_Settings.setValue("geometry", geometry());
    m_Settings.endGroup();

    int count = ui->tabWidget->count() - 1;
    for (int i = count; i >= 0; i--)
    {
        if (!closePage(i))
        {
            event->ignore();
            return;
        }
    }

    delete (StartupPageWidget::getWnd());
}

inline bool MainWindow::closePage(int index)
{
    Q_ASSERT(ui->tabWidget->count() > index);

    TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->widget(index));
    if (taskPage)
    {
        if (taskPage->hasChanged())
        {
            int ret = QMessageBox::question(this, tr("保存确认"), tr("是否保存当前任务？"), tr("保存"), tr("放弃"), tr("取消"));
            if (QMessageBox::DestructiveRole == ret)
            {
                return false;
            }
            else if (QMessageBox::AcceptRole == ret)
            {
                taskPage->saveChanges();
            }
        }

        ui->tabWidget->removeTab(index);

        delete taskPage;
        taskPage = NULL;
    }

    return true;
}

inline void MainWindow::addPage(AddMethod method, QString taskFile, QString taskName)
{
    if (AddMethod_Default == method && taskFile.isEmpty())
    {
        return;
    }
    else
    {
        StartupPageWidget startup(ui->tabWidget);
        startup.getTaskFile(method, taskFile, taskName);
    }

    if (!taskFile.isEmpty() && !hasOpened(taskFile))
    {
        int count = ui->tabWidget->count();
        TaskPageWidget *taskPage = new TaskPageWidget(count, taskFile, this);
        ui->tabWidget->addTab(taskPage, taskName);
        ui->tabWidget->setCurrentIndex(count);
        connect(taskPage, SIGNAL(changed(int)), SLOT(onChanged(int)));
        connect(taskPage, SIGNAL(maxShow(bool)), SLOT(showMax(bool)));
    }
}

bool MainWindow::hasOpened(const QString &taskFile)
{
    TaskPageWidget *taskPage = NULL;
    int count = ui->tabWidget->count();

    for (int i = 0; i < count; i++)
    {
        if ((taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->widget(i)))
                && taskPage->hasOpened(taskFile))
        {
            ui->tabWidget->setCurrentIndex(i);
            return true;
        }
    }

    return false;
}

void MainWindow::onChanged(int index)
{
    QString label = ui->tabWidget->tabText(index) + tr(" *");
    ui->tabWidget->setTabText(index, label);
    ui->tabWidget->setCurrentIndex(index);
    ui->saveAction->setEnabled(true);
}

void MainWindow::changeTab(int index)
{
    TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->widget(index));
    if (taskPage)
    {
       ui->saveAction->setEnabled(taskPage->hasChanged());
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    closePage(index);

    int count = ui->tabWidget->count(), n = 0;
    for (int i = 0; i < count; i++)
    {
        TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->widget(i));
        if (taskPage)
        {
            taskPage->setTabId(i);
            if (!taskPage->isEditing())
            {
                n++;
            }
        }
    }

    if (!count || n == count - 1)
    {
        setMinimumSize(MINI_WIDTH, MINI_HEIGHT);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setGeometry(this->geometry());
    }
}

void MainWindow::on_newAction_triggered()
{
    addPage(AddMethod_New);
}

void MainWindow::on_openAction_triggered()
{
    addPage(AddMethod_Open);
}

void MainWindow::on_saveAction_triggered()
{
    TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->currentWidget());
    if (taskPage && taskPage->hasChanged())
    {
        taskPage->saveChanges();
        ui->saveAction->setEnabled(false);

        int index = ui->tabWidget->currentIndex();
        QString label = ui->tabWidget->tabText(index);
        if (label.endsWith(" *"))
        {
            ui->tabWidget->setTabText(index, label.left(label.length() - 2));
        }
    }
}

void MainWindow::on_saveasAction_triggered()
{
    char *pTaskName = NULL;
    TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->currentWidget());

    if (taskPage && (pTaskName = taskPage->saveFile(Task_SaveAs)))
    {
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), tr("%1").arg(pTaskName));
        delete [] pTaskName;
        pTaskName = NULL;
    }
}

void MainWindow::on_importTemplatesAction_triggered()
{
    TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->currentWidget());
    if (taskPage)
    {
        taskPage->importTemplates();
    }
}

void MainWindow::on_manageTemplateAction_triggered()
{
//    TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->currentWidget());
//    if (taskPage)
//    {
//    }
}

void MainWindow::on_manageAlbumAction_triggered()
{
//    if (!m_albums)
//    {
//        m_albums = new AlbumManageDialog(this);
//    }
    //m_albums->show();

    QStringList albums;
//    int count = ui->tabWidget->count();

//    for (int i = 0; i < count; i++)
//    {
//        TaskPageWidget *taskPage = static_cast<TaskPageWidget *>(ui->tabWidget->widget(i));
//        QString album = taskPage->getAlbum();
//        if (!album.isEmpty())
//        {
//            albums << album;
//        }
//    }

    albums << tr("E:\\images\\album.xc") << tr("E:\\images\\李四相册.xc") << tr("E:\\images\\王五相册.xc") << tr("E:\\images\\张三相册.xc") << tr("E:\\images\\赵六.xc") << tr("E:\\images\\孙七.xc");

    m_albums->openWnd(albums);
}
