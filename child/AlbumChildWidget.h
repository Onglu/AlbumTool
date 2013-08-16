#ifndef ALBUMCHILDWIDGET_H
#define ALBUMCHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"

#define PHOTOS_NUMBER           6
#define INVALID_PHOTO_INDEX     -1

class TemplateChildWidget;
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
                     const QVariantList &photosInfo,
                     const QString &tmplFile,
                     const QVariantList &photoLayers,
                     TaskPageWidget *parent = 0);
    ~AlbumChildWidget();

    /* If belongings is empty, so it indicates that is a delete operation */
    void changeTemplate(const QVariantMap &belongings = QVariantMap());

    void changePhoto(const QString &layerId,
                     QRect rect,
                     qreal opacity = 1,
                     qreal angle = 0);

    bool removePhoto(const QString &fileName);

    void changeBanners(void);

    void clearBanners(void);

    void switchView(void);

    void replace(const QString &current, const QVariantMap &belongings);

    void open(ChildWidgetsMap &widgetsMap);

    /* return the number of photos that are put in per album page */
    int output(const QString &dir);

    TemplateChildWidget *getTmplWidget(void) const;

    DraggableLabel &getTmplLabel(void){return *m_tmplLabel;}

    const QString &getTmplFile(void) const {return m_tmplFile;}

    const QVariantMap &getChanges(void);

    const QVariantMap &getData(void) const
    {
        QVariantMap belongings = m_tmplLabel->getBelongings();
        return belongings["page_data"].toMap();
    }

    uchar getLocations(void) const
    {
        return m_locations[PORTRAIT_PICTURE] + m_locations[LANDSCAPE_PICTURE];
    }

    /* Get the available photo locations of this abum */
    uchar getLocations(uchar locations[]) const;

    QSize getSize(void) const;

    const QVariantList &getLayers(void) const;

    DraggableLabels &getPhotoLabels(void){return m_photoLabels;}

    bool setPhotoInfo(DraggableLabel *picLabel, QVariantMap info = QVariantMap());

    void resetPhotosInfo(void);

    QVariantList &getPhotosInfo(void){return m_photosInfo;}

    const QStringList &getPhotosList(void);

    int getPhotoIndex(const QString &fileName);

    int getPhotosNum(void) const;

    /* Get the total used times of ervery photo in the present album page */
    int getTotalUsedTimes(void) const;

    static bool addUsedRecord(QVariantList &records, const QString &layerId);

    static void removeUsedRecord(QVariantList &records, const QString &layerId);

protected:
    DraggableLabel *setPictureLabel(QLabel *label);

    bool meetDragDrop(QDropEvent *event);

    void dropEvent(QDropEvent *event);

private:
    void addBanner(QString banner, QString tip);
    void addBanners(int count, QString banner, QString tip);

    void setupWidgets(const QVariantList &photosInfo = QVariantList(),
                      const QString &tmplFile = QString(),
                      const QVariantList &photoLayers = QVariantList());

    void showPhotos(bool visible);

    Ui::AlbumChildWidget *ui;
    bool m_tmplVisible, m_locked;
    uchar m_locations[2];   // 0: portrait(Verti), 1: landscape(Hori)

    DraggableLabels m_photoLabels;
    DraggableLabel *m_tmplLabel;

    QStringList m_photosList;
    QVariantList m_photosInfo;

    QString m_tmplFile;
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
