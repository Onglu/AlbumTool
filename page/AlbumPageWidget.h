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

    // 加载模板图层
    bool loadLayers(const AlbumChildWidget &album);

    // 清除模板图层
    void clearLayers(void);

    // 加载相册图片
    int loadPhotos(AlbumChildWidget &album,
                   QVariantList &photosInfo,
                   int totalTimes,
                   const QString &savePath = QString());

    // 移除相册图片
    void removePhoto(const QString &fileName);

    //BgdLayer *getBgdLayer(void) const {return m_bgdLabel;}

    //const LabelsVector &getLayerLabels(void){return m_layerLabels;}

    void sort(QVariantList &records);

    void replace(AlbumChildWidget &album,
                 const ThumbChildWidget *thumb,
                 PhotoLayer *label = NULL);

    // 合成相册图片
    void compose(int locations = -1, const QString &saveFile = QString());

private:
    // 获取模板图层显示框
    QRect getLocation(const QString &layerId, const QString &maskFile) const;

    // 获取模板图层显示框
    QRect getLocation(const QVariantMap &frame) const;

    // 导出相册页上的所有图片
    void exportPhotos(AlbumChildWidget &album,
                      PhotoLayer &layer,
                      const QString &layerId,
                      const QString &savePath);

    // 导出相册页上的指定图片
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
