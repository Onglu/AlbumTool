#ifndef TEMPLATECHILDWIDGET_H
#define TEMPLATECHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"
#include "wrapper/utility.h"

#define TEMP_FILE       "/DDECF6B7F103CFC11B2.png"
#define PKG_FMT         ".xcmb"
#define PKG_PASSWORD    "123123"

typedef PictureProxyWidget TemplateProxyWidget;

class TemplateChildWidget : public PictureChildWidget
{
    Q_OBJECT

public:
    explicit TemplateChildWidget(int index,
                                 const QString &tmplFile,
                                 int usedTimes = 0,
                                 TaskPageWidget *parent = 0);
    virtual ~TemplateChildWidget();

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

    uchar getPageType(void){return (uchar)m_bases["pagetype"].toInt();}

    const QVariantList &getTags(void){return m_tags;}

    const QVariantList &getLayers(void){return m_layers;}

    const uchar *getLocations(void){return m_locations;}

    enum ZipUsage{ZipUsageCompress,
                  ZipUsageAppend,
                  ZipUsageRemove,
                  ZipUsageUncompress,
                  ZipUsageList,
                  ZipUsageRead,
                  ZipUsageEncrypt,
                  ZipUsageDecrypt
                 };

    static void useZip(ZipUsage usage, const QString &arguments, bool block = false);

    //static void parse(const QVariantMap &data);

    const QVariantMap &getData(void) const {return m_data;}

private slots:
    void ok(const QString &pkgFile);

    void processFinished(int, QProcess::ExitStatus);

private:
    void loadPictures();

    CryptThread m_parser;
    static QProcess m_tmaker;
    static ZipUsage m_usage;

    QString m_tmplFile, m_tmplPic, m_tmplDir, m_currFile;
    QVariantMap m_data, m_belongings, m_bases, m_size, m_pictures;
    QVariantList m_tags, m_layers;
    uchar m_locations[2];   // 0: landscape(Hori), 1: portrait(Verti)
};



#endif // TEMPLATECHILDWIDGET_H
