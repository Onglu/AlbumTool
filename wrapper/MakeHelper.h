#ifndef MAKEHELPER_H
#define MAKEHELPER_H

#include <QThread>
#include "wrapper/BgdLayer.h"

class AlbumChildWidget;

class MakeHelper : public QThread
{
    Q_OBJECT

public:
    explicit MakeHelper(AlbumChildWidget *page, QObject *parent = 0);
    
signals:
    
public slots:
    
protected:
    void run();

private:
    //bool updateLayers(const QStringList &photosList);

    AlbumChildWidget *m_page;
};

#endif // MAKEHELPER_H
