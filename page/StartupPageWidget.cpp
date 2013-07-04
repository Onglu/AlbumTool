#include "StartupPageWidget.h"
#include "MainWindow.h"
#include "parser/FileParser.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QtSql>
#include "wrapper/utility.h"    // for test

MainWindow *StartupPageWidget::m_pMW = NULL;

StartupPageWidget::StartupPageWidget(Qt::WindowFlags f, QSize fixed, QWidget *parent) : QWidget(parent, f)
{
    if (!parent)    // None parent window
    {
        setWindowTitle("相册编辑工具");
        setWindowIcon(QIcon(":/images/logo.png"));
        setFixedSize(fixed);

        QSize btnSize(20, 20);
        m_pCloseBtn = new QPushButton(this);
        m_pCloseBtn->setFixedSize(btnSize);
        m_pCloseBtn->setIconSize(btnSize);
        m_pCloseBtn->setFlat(true);
        m_pCloseBtn->setIcon(QIcon(":/images/close.png"));
        m_pCloseBtn->move(fixed.width() - btnSize.width(), 0);
        connect(m_pCloseBtn, SIGNAL(clicked()), SLOT(openMw()));
    }

    m_pNewBtn = new QPushButton(tr("新建任务"));
    m_pNewBtn->setFixedSize(75, 23);
    connect(m_pNewBtn, SIGNAL(clicked()), SLOT(newTask()));

    m_pOpenBtn = new QPushButton(tr("打开任务"), this);
    m_pOpenBtn->setFixedSize(75, 23);
    connect(m_pOpenBtn, SIGNAL(clicked()), SLOT(openTask()));

    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->setAlignment(Qt::AlignCenter);
    hbl->addWidget(m_pNewBtn);
    hbl->addWidget(m_pOpenBtn);
    setLayout(hbl);

    initTaskDir();


//    QSqlQuery query;
    //QString sql = tr("select id from tproperty where ptype='种类' and name='婚纱'");
    //QString sql = tr("delete template, template_property from template left join template_property on template_property.templateid=template.id where template.id=1");
    //QString sql = tr("delete from template inner join template_property on template_property.templateid=template.id where template.id=1");
    //QString sql = tr("delete from template where id=1");
    //query.exec(sql);
//    while (query.next())
//    {
//        int pid = query.value(0).toInt();
//        qDebug() << __FILE__ << __LINE__ << "find:" << pid;
//        sql = tr("insert into template_property values(%1,%2)").arg(1).arg(pid);
//        query.exec(sql);
//    }

//    QString photoName;
//    Converter::getFileName("C:\\Users\\Onglu\\Desktop\\test\\Copied_kk.png", photoName, true);
//    qDebug() << __FILE__ << __LINE__ << photoName;
}

StartupPageWidget::~StartupPageWidget()
{
    m_Settings.beginGroup("Location");
    m_Settings.setValue("task_dir", m_taskDir);
    m_Settings.endGroup();
}

inline void StartupPageWidget::initTaskDir()
{
    m_Settings.beginGroup("Location");
    m_taskDir = m_Settings.value("task_dir").toString();
    if (!QDir(m_taskDir).exists())
    {
        m_taskDir = QDir::homePath() + "/";
    }
    m_Settings.endGroup();
}

const QString &StartupPageWidget::getTaskFile(uchar mode, QString &taskFile, QString &taskName)
{
    QString srcFile;

    if (Task_New == mode)
    {
        taskFile = QFileDialog::getSaveFileName(parentWidget(), tr("新建任务"), m_taskDir, tr("相册任务(*.xcrw)"));
        FileParser(taskFile).clear();
    }

    if (Task_Open == mode)
    {
        taskFile = QFileDialog::getOpenFileName(parentWidget(), tr("打开任务"), m_taskDir, tr("相册任务(*.xcrw)"));
    }

    if (Task_SaveAs == mode)
    {
        srcFile = taskFile;
        taskFile = QFileDialog::getSaveFileName(parentWidget(), tr("另存为..."), srcFile, tr("相册任务(*.xcrw)"));
        if (taskFile.isEmpty() || !m_pMW || m_pMW->hasOpened(QDir::toNativeSeparators(taskFile)))
        {
            return (taskFile = srcFile);
        }

        FileParser fp(taskFile);
        if (!fp.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(parentWidget(), tr("另存失败"), tr("请检查目标文件是否已经被锁定！"), tr("确定"));
            return (taskFile = srcFile);
        }
    }

    if (!taskFile.isEmpty())
    {
        taskFile = QDir::toNativeSeparators(taskFile);
        m_taskDir = taskFile.left(taskFile.lastIndexOf(QDir::separator()) + 1);
        taskName = taskFile.right(taskFile.length() - m_taskDir.length());
    }

    return taskFile;
}

inline void StartupPageWidget::openMw(QString taskFile, QString taskName)
{
    if (!m_pMW)
    {
        m_pMW = new MainWindow(taskFile, taskName);
    }

    m_pMW->show();
    close();
}

void StartupPageWidget::newTask()
{
    QString taskFile, taskName;
    getTaskFile(Task_New, taskFile, taskName);
    openMw(taskFile, taskName);
}

void StartupPageWidget::openTask()
{
    QString taskFile, taskName;
    getTaskFile(Task_Open, taskFile, taskName);
    openMw(taskFile, taskName);
}
