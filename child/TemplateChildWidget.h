#ifndef TEMPLATECHILDWIDGET_H
#define TEMPLATECHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"
#include "wrapper/utility.h"

typedef PictureProxyWidget TemplateProxyWidget;
class TemplateChildWidget;

namespace TemplatesSql
{
    class SqlThread : public QThread
    {
        Q_OBJECT

    public:
        SqlThread(TemplateChildWidget *widget = 0) : m_search(false), m_widget(widget){}

        void bindData(const QVariantMap &data, bool search = false)
        {
            m_data = data;
            m_search = search;
        }

    signals:

    protected:
        void run();

    private:
        bool existing(void);

        bool m_search;
        QVariantMap m_data;
        TemplateChildWidget *m_widget;
    };
}

class TemplateChildWidget : public PictureChildWidget/*, public SqlHelper*/
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
                            int usedTimes = 0);

    const QVariantMap &getChanges(void);

    bool getTmplFile(QString &tmplFile, bool pkgFile = true);

    static bool moveTo(QString &fileName, QString dirName, bool overwrite = true);

    //const QString &getTmplFile(void){return m_tmplFile;}

    //const QString &getTmplPic(void){return m_tmplPic;}

    //const QString &getTmplDir(void){return m_tmplDir;}

    //const QVariantMap &getBases(void){return m_bases;}

    QSize getSize(void) const
    {
        //return m_size.size() ? QSize(m_size["width"].toInt(), m_size["height"].toInt()) : QSize();
        QVariantMap size = m_data["size"].toMap();
        return QSize(size["width"].toInt(), size["height"].toInt());
    }

    //uchar getPageType(void){return (uchar)m_bases["pagetype"].toInt();}

    const QVariantList &getTags(void) const {return m_data["tags"].toList();}

    //const QVariantList &getLayers(void){return m_layers;}

    const uchar *getLocations(void)
    {
        memset(m_locations, 2, 0);

        if (m_data.contains("landscapeCount"))
        {
            m_locations[0] = (uchar)m_data["landscapeCount"].toUInt();
        }

        if (m_data.contains("portraitCount"))
        {
            m_locations[1] = (uchar)m_data["portraitCount"].toUInt();
        }

        return m_locations;
    }

    enum ZipUsage{ZipUsageCompress,
                  ZipUsageAppend,
                  ZipUsageRemove,
                  ZipUsageUncompress,
                  ZipUsageList,
                  ZipUsageRead,
                  ZipUsageEncrypt,
                  ZipUsageDecrypt
                 };

    static void useZip(QProcess &tmaker,
                       ZipUsage usage,
                       const QString &arguments,
                       bool block = false);

    //static void parse(const QVariantMap &data);

    const QVariantMap &getData(void) const {return m_data;}

    bool isCover(void) const {return 1 == m_data["pagetype"].toInt();}

    bool match(QVariantMap tags);

    void remove(void){remove(getId());}

private slots:
    void processFinished(int, QProcess::ExitStatus);

private:
    void loadPicture(QVariantMap &data, QString tmplPic = QString());

    void loadPictures();

    int getId(void){return SqlHelper::getId(tr("select id from template where fileurl='%1'").arg(m_tmplPic));}

    void remove(int tid);

    CryptThread m_parser;
    QProcess m_tmaker;

    QString m_tmplFile, m_tmplPic, m_currFile;
    QVariantMap m_data, m_pictures, /*m_belongings, m_bases, m_size*/;
    //QVariantList m_tags/*, m_layers*/;
    uchar m_locations[2];   // 0: landscape(Hori), 1: portrait(Verti)

    TemplatesSql::SqlThread m_sql;
    friend class TemplatesSql::SqlThread;
};



#endif // TEMPLATECHILDWIDGET_H
