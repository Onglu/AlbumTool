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

    TemplateChildWidget(const QString &tmplFile, DraggableLabel *label);

    const QVariantMap &getChanges(void);

    const QString &getTmplFile(void){return m_tmplFile;}

    const QString &getTmplPic(void){return m_tmplPic;}

    static QSize getSize(const QVariantMap &data)
    {
        QVariantMap size = data["size"].toMap();
        return QSize(size["width"].toInt(), size["height"].toInt());
    }

    //uchar getPageType(void){return (uchar)m_bases["pagetype"].toInt();}

    //const QVariantList &getTags(void) const {return m_data["tags"].toList();}

    static const QVariantList &getLayers(const QVariantMap &data){return data["layers"].toList();}

    static const uchar *getLocations(const QVariantMap &data, uchar locations[])
    {
        memset(locations, 2, 0);

        if (data.contains("landscapeCount"))
        {
            locations[0] = (uchar)data["landscapeCount"].toUInt();
        }

        if (data.contains("portraitCount"))
        {
            locations[1] = (uchar)data["portraitCount"].toUInt();
        }

        return locations;
    }

    //const QVariantMap &getData(void) const {return m_data;}

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

    static bool moveTo(QString &fileName, QString dirName, bool overwrite = true);

    static void parseTest(const QVariantMap &data);

    //bool isCover(void) const {return 1 == m_data["pagetype"].toInt();}

    void remove(void){remove(getId());}

    const QVariantMap &loadPictures(void);

protected slots:
    void onAccept(const QVariantMap &belongings);

private slots:
    void processFinished(int, QProcess::ExitStatus);

private:
    bool getTmplPic(QString &tmplPic);

    void loadPicture(QVariantMap &data, QString tmplPic = QString());

    int getId(void){return SqlHelper::getId(tr("select id from template where fileurl='%1'").arg(m_tmplPic));}

    void remove(int tid);

    CryptThread m_parser;
    QProcess m_tmaker;

    QString m_tmplFile, m_tmplPic, m_currFile;
    QVariantMap m_pictures;
    uchar m_locations[2];   // 0: landscape(Hori), 1: portrait(Verti)

    TemplatesSql::SqlThread m_sql;
    friend class TemplatesSql::SqlThread;
};



#endif // TEMPLATECHILDWIDGET_H
