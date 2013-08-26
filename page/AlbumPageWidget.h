#ifndef ALBUMPAGEWIDGET_H
#define ALBUMPAGEWIDGET_H

#include <QWidget>
#include "wrapper/BgdLayer.h"
//#include "child/TemplateChildWidget.h"

class AlbumChildWidget;
class TemplateChildWidget;
class ThumbChildWidget;
class EditPageWidget;

class AlbumPageWidget : public QWidget
{
public:
    explicit AlbumPageWidget(PhotoLayer::VisiableImgType type, QWidget *parent = 0);

    bool loadLayers(const AlbumChildWidget &album);

    void clearLayers(void);

    int loadPhotos(AlbumChildWidget &album,
                   QVariantList &photosInfo,
                   int totalTimes,
                   const QString &savePath = QString());

    void removePhoto(const QString &fileName);

    //BgdLayer *getBgdLayer(void) const {return m_bgdLabel;}

    //const LabelsVector &getLayerLabels(void){return m_layerLabels;}

    void sort(QVariantList &records);

    void replace(AlbumChildWidget &album,
                 const ThumbChildWidget *thumb,
                 PhotoLayer *label = NULL);

    void compose(int locations = -1, const QString &saveFile = QString());

private:
    QRect getLocation(const QString &layerId, const QString &maskFile) const;

    QRect getLocation(const QVariantMap &frame) const;

    void exportPhotos(AlbumChildWidget &album,
                      PhotoLayer &layer,
                      const QString &layerId,
                      const QString &savePath);

    void exportPhoto(const QImage &image, const QString &savePath, const QString &fileName);

    //PicturesMap m_pictures;
    QVariantList m_layers, m_photoLayers;
    LabelsVector m_layerLabels;
    BgdLayer *m_bgdLabel;

    TemplateChildWidget *m_tmplWidget;

    friend class AlbumChildWidget;
    friend class EditPageWidget;
};

#endif // ALBUMPAGEWIDGET_H
