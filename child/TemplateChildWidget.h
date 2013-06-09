#ifndef TEMPLATECHILDWIDGET_H
#define TEMPLATECHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"

typedef PictureProxyWidget TemplateProxyWidget;

class TemplateChildWidget : public PictureChildWidget
{
    Q_OBJECT

public:
    explicit TemplateChildWidget(int index,
                                 const QString &tmplFile,
                                 int usedTimes = 0,
                                 TaskPageWidget *parent = 0);

    static void setTemplate(DraggableLabel &label,
                            const QString &tmplPic,
                            const QString &tmplFile,
                            int cover = 0,
                            int used_times = 0);

    const QVariantMap &getChanges(void);

    const QString &getTmplFile(void){return m_tmplFile;}

    const QString &getTmplPic(void){return m_tmplPic;}

    const QString &getTmplDir(void){return m_tmplDir;}

    const QVariantMap &getBases(void){return m_bases;}

    QSize getSize(void)
    {
        return m_size.size() ? QSize(m_size["width"].toInt(), m_size["height"].toInt()) : QSize();
    }

    const QVariantList &getTags(void){return m_tags;}

    const QVariantList &getLayers(void){return m_layers;}

    const uchar *getLocations(void){return m_locations;}

private:

    QString m_tmplFile, m_tmplPic, m_tmplDir;
    QVariantMap m_belongings, m_bases, m_size;
    QVariantList m_tags, m_layers;
    uchar m_locations[2];   // 0: landscape(Hori), 1: portrait(Verti)
};

#endif // TEMPLATECHILDWIDGET_H
