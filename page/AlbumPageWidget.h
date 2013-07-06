#ifndef ALBUMPAGEWIDGET_H
#define ALBUMPAGEWIDGET_H

#include <QWidget>
#include "wrapper/BgdLayer.h"

class AlbumChildWidget;
class EditPageWidget;

class AlbumPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumPageWidget(PhotoLayer::VisiableImgType type, QWidget *parent = 0);

    bool loadLayers(const AlbumChildWidget &album);

    void clearLayers(void);

    bool loadPhoto(int index,
                   const QString &photoFile,
                   qreal angle = 0,
                   Qt::Axis axis = Qt::ZAxis);

    int loadPhotos(const QStringList &photosList);

    //BgdLayer *getBgdLayer(void) const {return m_bgdLabel;}

    //const LabelsVector &getLayerLabels(void){return m_layerLabels;}

    void replace(const QString &current, const QString &replaced);

    void compose(int count = -1, const QString &fileName = QString());

    void showPage(bool visiable);

signals:
    
private slots:
    
private:
    QVariantList m_layers, m_photoLayers;
    LabelsVector m_layerLabels;
    BgdLayer *m_bgdLabel;

    friend class AlbumChildWidget;
    friend class EditPageWidget;
};

#endif // ALBUMPAGEWIDGET_H
