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
        bool existing(const QString &pageId);

        bool m_search;
        QVariantMap m_data;
        TemplateChildWidget *m_widget;
    };
}

class TemplateChildWidget : public PictureChildWidget
{
    Q_OBJECT

public:
    explicit TemplateChildWidget(int index,
                                 const QString &file,
                                 int usedTimes = 0,
                                 TaskPageWidget *parent = 0);

    TemplateChildWidget(const QString &file, DraggableLabel *label);

    const QVariantMap &getChanges(void);

    const QString &getTmplFile(void){return m_tmplFile;}

    bool getTmplPic(QString &tmplPic);

    static bool isCover(const QVariantMap &data)
    {
        return (1 == data["pagetype"].toInt());
    }

    static QSize getSize(const QVariantMap &data)
    {
        QVariantMap size = data["size"].toMap();
        return QSize(size["width"].toInt(), size["height"].toInt());
    }

    //static const QVariantList &getLayers(const QVariantMap &data){return data["layers"].toList();}

    static const uchar *getLocations(const QVariantMap &data, uchar locations[])
    {
        memset(locations, 2, 0);

        if (data.contains("portraitCount"))
        {
            locations[PORTRAIT_PICTURE] = (uchar)data["portraitCount"].toUInt();
        }

        if (data.contains("landscapeCount"))
        {
            locations[LANDSCAPE_PICTURE] = (uchar)data["landscapeCount"].toUInt();
        }

        return locations;
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

    static bool moveTo(QString &fileName, QString dirName, bool overwrite = true);

    static void parseTest(const QVariantMap &data);

    void remove(void){remove(getId());}

    const QVariantMap &loadPictures(void);

protected slots:
    void onAccept(const QVariantMap &belongings);

private slots:
    void processFinished(int, QProcess::ExitStatus);

private:
    void loadPicture(QVariantMap &data, QString tmplPic = QString());

    int getId(void);

    void remove(int tid);

    CryptThread m_parser;
    QProcess m_tmaker;

    QString m_tmplFile, m_tmplPic, m_currFile;
    QVariantMap m_pictures;

    TemplatesSql::SqlThread m_sql;
    friend class TemplatesSql::SqlThread;
};



#endif // TEMPLATECHILDWIDGET_H
