#-------------------------------------------------
#
# Project created by QtCreator 2013-03-27T16:31:58
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Album
TEMPLATE = app
DEFINES += FROM_PACKAGE
#TRANSLATIONS ＋＝ ./temp/zh_CN.ts

#QTPLUGIN += qsqlite

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
    wrapper/BgdLayer.cpp \
    page/AlbumPageWidget.cpp \
    LoadingDialog.cpp \
    manage/AlbumTaskWidget.cpp \
    manage/AlbumManageDialog.cpp \
    manage/AlbumInfoWidget.cpp \
    manage/UserInfoDialog.cpp

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
    wrapper/BgdLayer.h \
    page/AlbumPageWidget.h \
    LoadingDialog.h \
    manage/AlbumTaskWidget.h \
    manage/AlbumManageDialog.h \
    manage/AlbumInfoWidget.h \
    manage/UserInfoDialog.h \
    manage/UserInfoDialog.h

FORMS    += \
    MainWindow.ui \
    PreviewDialog.ui \
    child/PhotoChildWidget.ui \
    child/AlbumChildWidget.ui \
    page/TemplatePageWidget.ui \
    page/TaskPageWidget.ui \
    page/EditPageWidget.ui \
    manage/AlbumTaskWidget.ui \
    manage/AlbumManageDialog.ui \
    manage/AlbumInfoWidget.ui \
    manage/UserInfoDialog.ui

RESOURCES += \
    images.qrc

OTHER_FILES += \
    records.txt

RC_FILE += app.rc
