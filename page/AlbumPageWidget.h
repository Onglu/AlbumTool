#ifndef ALBUMPAGEWIDGET_H
#define ALBUMPAGEWIDGET_H

#include <QWidget>

class AlbumPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AlbumPageWidget(QWidget *parent = 0);
    
signals:
    
public slots:
    
private:
    QPixmap m_tmpl;
};

#endif // ALBUMPAGEWIDGET_H
