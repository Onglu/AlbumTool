#-------------------------------------------------
#
# Project created by QtCreator 2013-03-27T16:31:58
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Album
TEMPLATE = app
DEFINES += FROM_PACKAGE
#TRANSLATIONS ＋＝ ./temp/zh_CN.ts

SOURCES += main.cpp\
    MainWindow.cpp \
    PreviewDialog.cpp \
    child/ThumbChildWidget.cpp \
    child/TemplateChildWidget.cpp \
    child/PictureChildWidget.cpp \
    child/PhotoChildWidget.cpp \
    child/AlbumChildWidget.cpp \
    page/TemplatePageWidget.cpp \
    page/TaskPageWidget.cpp \
    page/StartupPageWidget.cpp \
    page/EditPageWidget.cpp \
    parser/json.cpp \
    parser/FileParser.cpp \
    proxy/PictureProxyWidget.cpp \
    wrapper/utility.cpp \
    wrapper/DraggableLabel.cpp \
    wrapper/PictureGraphicsScene.cpp \
    wrapper/PictureLabel.cpp \
    wrapper/PhotoLayer.cpp \
    SqlEngine.cpp \
    wrapper/BgdLayer.cpp

HEADERS  += \
    MainWindow.h \
    PreviewDialog.h \
    child/ThumbChildWidget.h \
    child/TemplateChildWidget.h \
    child/PictureChildWidget.h \
    child/PhotoChildWidget.h \
    child/AlbumChildWidget.h \
    page/TemplatePageWidget.h \
    page/TaskPageWidget.h \
    page/StartupPageWidget.h \
    page/EditPageWidget.h \
    parser/json.h \
    parser/FileParser.h \
    proxy/PictureProxyWidget.h \
    wrapper/utility.h \
    wrapper/DraggableLabel.h \
    wrapper/PictureGraphicsScene.h \
    wrapper/PictureLabel.h \
    wrapper/PhotoLayer.h \
    defines.h \
    events.h \
    SqlEngine.h \
    wrapper/BgdLayer.h

FORMS    += \
    MainWindow.ui \
    PreviewDialog.ui \
    child/PhotoChildWidget.ui \
    child/AlbumChildWidget.ui \
    page/TemplatePageWidget.ui \
    page/TaskPageWidget.ui \
    page/EditPageWidget.ui

RESOURCES += \
    images.qrc

OTHER_FILES += \
    records.txt

RC_FILE += app.rc
