#ifndef ALBUMCHILDWIDGET_H
#define ALBUMCHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"

#define PHOTOS_NUMBER  6

typedef QVector<QString> AlbumPhotos;
typedef QList<DraggableLabel *> DraggableLabels;

namespace Ui {
    class AlbumChildWidget;
}

class AlbumChildWidget : public PictureChildWidget
{
    Q_OBJECT

public:
    explicit AlbumChildWidget(int index, TaskPageWidget *parent = 0);
    AlbumChildWidget(int index,
                     const QStringList &photosList,
                     const QString &tmplFile,
                     TaskPageWidget *parent = 0);
    ~AlbumChildWidget();

    DraggableLabel &getTmplLabel(void){return *m_tmplLabel;}
    DraggableLabels &getPhotoLabels(void){return m_photoLabels;}
    QStringList &getPhotosList(void){return m_photosList;}

    /* If tmplFile is empty, so it indicates that is a delete operation */
    void changeTemplate(const QString &tmplFile = QString(),
                        const QPixmap &tmplPic = QPixmap(),
                        const QVariantMap &belongings = QVariantMap());

    void setViewsList(const AlbumPhotos &photosVector,
                      const QString &tmplFile = QString("")/* Default indicates that doesn't change the current template */);
    void getViewsList(AlbumPhotos &photosVector,
                      QString &tmpl,
                      bool pic = true/* true: get template picture file name, otherwise get its data file name */);

    QVariantList &getLayersList(void){return m_layers;}

    QSize getCanvasSize(void) const {return m_canvasSize;}    // returns the real size about the current canvas

    void switchView(bool enter);

    void open(ChildWidgetsMap &widgetsMap);

    const QVariantMap &getChanges(void);

    uchar getLocations(void) const {return m_locations[0] + m_locations[1];}

    void clearBanners(void);

signals:

protected:
    bool meetDragDrop(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    const QString &readTmplData(const QString &tmplFile, QString &tmplPic); // returns the template picture file name

    void changeBanners(void);
    void addBanner(QString banner, QString tip);
    void addBanners(int count, QString banner, QString tip);

    void setupWidgets(const QStringList &photosList = QStringList(), const QString &tmplPic = QString());
    void showPhotosView(bool visible);

    Ui::AlbumChildWidget *ui;
    QSize m_thumbSize;
    bool m_tmplVisible, m_locked;

    DraggableLabels m_photoLabels;
    DraggableLabel *m_tmplLabel;

    QStringList m_photosList;       // item format: file, doesn't contain empty items
    AlbumPhotos m_photosVector;     // value format: file|angle|axis, contains empty items

    /* Tempalte data */
    QString m_tmplFile;
    QVariantMap m_bases;
    QSize m_canvasSize;
    QVariantList m_tags, m_layers;
    uchar m_locations[2];   // 0: landscape(Hori), 1: portrait(Verti)
};

class AlbumProxyWidget : public PictureProxyWidget
{
    Q_OBJECT

public:
    AlbumProxyWidget(AlbumChildWidget *albumWidget) :
        PictureProxyWidget(albumWidget), m_pChildWidget(albumWidget)
    {
        Q_ASSERT(m_pChildWidget);
    }

    bool excludeItem(const QString &picFile);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    AlbumChildWidget *m_pChildWidget;
};

#endif // ALBUMCHILDWIDGET_H
