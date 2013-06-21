#include <QApplication>
#include <QDesktopWidget>
#include <QTextCodec>
#include "page/StartupPageWidget.h"

QRect g_appRect;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Jizhiyou");
    QCoreApplication::setOrganizationDomain("jizhiyou.com");
    QCoreApplication::setApplicationName("Album");

    QTextCodec *codec = QTextCodec::codecForName("utf-8"); // utf-8, GB18030
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForLocale(/*codec*/ QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(/*codec*/ QTextCodec::codecForLocale());

    g_appRect = QApplication::desktop()->availableGeometry();
    //g_rcDesktop = app.desktop()->availableGeometry();

    StartupPageWidget s(Qt::FramelessWindowHint, QSize(320, 200));
    s.show();

    return app.exec();
}
