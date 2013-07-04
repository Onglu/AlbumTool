#include "MakeHelper.h"
#include "child/AlbumChildWidget.h"
#include "child/TemplateChildWidget.h"

MakeHelper::MakeHelper(AlbumChildWidget *page, QObject *parent) :
    QThread(parent),
    m_page(page)
{
//    Q_ASSERT(m_page);

//    m_bgdLabel = new BgdLayer(m_page);

//    for (int i = 0; i < PHOTOS_NUMBER; i++)
//    {
//        m_layerLabels.insert(i, new PhotoLayer(m_page));
//    }
}

void MakeHelper::run()
{

}
