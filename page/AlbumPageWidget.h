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

//    bool loadPhoto(int index,
//                   QVariantMap &record,
//                   const QString &fileName,
//                   qreal angle = 0,
//                   Qt::Axis axis = Qt::ZAxis,
//                   int id = -1
//                   );

    //int loadPhotos(const QVariantList &photosInfo);

    int loadPhotos(QVariantList &photosInfo,
                   int totalTimes,
                   int photosNum,
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
