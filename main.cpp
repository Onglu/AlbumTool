#include <QApplication>
#include <QDesktopWidget>
#include <QTextCodec>
#include <QtPlugin>
#include "page/StartupPageWidget.h"
#include "wrapper/utility.h"

//Q_IMPORT_PLUGIN(qsqlite)

QRect g_appRect;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("utf-8"); // utf-8, GB18030
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

    QCoreApplication::setOrganizationName("Jizhiyou");
    QCoreApplication::setOrganizationDomain("jizhiyou.com");
    QCoreApplication::setApplicationName("Album");

    g_appRect = QApplication::desktop()->availableGeometry();
    //g_rcDesktop = app.desktop()->availableGeometry();

    StartupPageWidget s(Qt::FramelessWindowHint, QSize(320, 200));
    s.show();

    return app.exec();
}
