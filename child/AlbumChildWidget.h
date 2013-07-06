#ifndef ALBUMCHILDWIDGET_H
#define ALBUMCHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"

#define PHOTOS_NUMBER       6
#define PHOTO_ATTRIBUTES    4

class MakeHelper;
class TemplateChildWidget;
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
                     const QVariantList &photoLayers,
                     TaskPageWidget *parent = 0);
    ~AlbumChildWidget();

    /* If belongings is empty, so it indicates that is a delete operation */
    void changeTemplate(const QVariantMap &belongings = QVariantMap());

    void changePhoto(const QString &photoName,
                     QRect rect,
                     qreal opacity = 1,
                     qreal angle = 0);

    void setPhotosVector(const AlbumPhotos &photosVector)
    {
        m_photosVector = photosVector;
        changeBanners();
    }

    void switchView(bool enter);

    void clearBanners(void);

    void open(ChildWidgetsMap &widgetsMap);

    bool output(const QString &dir);

    TemplateChildWidget *getTmplWidget(void) const;
    DraggableLabel &getTmplLabel(void){return *m_tmplLabel;}
    DraggableLabels &getPhotoLabels(void){return m_photoLabels;}

    AlbumPhotos &getPhotosVector(void){return m_photosVector;}
    QStringList &getPhotosList(void){return m_photosList;}
    const QString &getTmplFile(void){return m_tmplFile;}

    const QVariantMap &getChanges(void);

    const QVariantMap &getData(void) const
    {
        QVariantMap belongings = m_tmplLabel->getBelongings();
        return belongings["page_data"].toMap();
    }

    uchar getLocations(void) const {return m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE];}

    uchar getLocations(uchar locations[]) const;

    QSize getSize(void) const;

    const QVariantList &getLayers(void) const;

signals:

protected:
    bool meetDragDrop(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void changeBanners(void);
    void addBanner(QString banner, QString tip);
    void addBanners(int count, QString banner, QString tip);

    void setupWidgets(const QStringList &photosList = QStringList(),
                      const QString &tmplFile = QString(),
                      const QVariantList &photoLayers = QVariantList());

    void showPhotos(bool visible);

    Ui::AlbumChildWidget *ui;
    bool m_tmplVisible, m_locked;
    QSize m_thumbSize;

    DraggableLabels m_photoLabels;
    DraggableLabel *m_tmplLabel;

    QStringList m_photosList;       // item format: filename with path, not contains the empty items
    AlbumPhotos m_photosVector;     // value format: file|angle|axis, contains empty items

    /* Tempalte data */
    QString m_tmplFile;
    uchar m_locations[2];   // 0: portrait(Verti), 1: landscape(Hori)

    MakeHelper *m_maker;
    TemplateChildWidget *m_tmpl;

    friend class MakeHelper;
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
