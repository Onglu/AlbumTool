#ifndef TEMPLATECHILDWIDGET_H
#define TEMPLATECHILDWIDGET_H

#include "child/PictureChildWidget.h"
#include "proxy/PictureProxyWidget.h"
#include "wrapper/utility.h"

#define LOAD_FROM_MEMORY    1

#if !LOAD_FROM_MEMORY
typedef QMap<QString, QPixmap *> PicturesMap;
#endif

typedef PictureProxyWidget TemplateProxyWidget;
class TemplateChildWidget;

namespace TemplateEngine
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
        int existing(/*const QString &pageId*/) const;

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

    TemplateChildWidget(const QString &file,
                        DraggableLabel *label,
                        TaskPageWidget *parent = 0);

    const QVariantMap &getChanges(void);

    // 获取模板文件
    const QString &getTmplFile(void) const {return m_tmplFile;}

    // 获取模板图片
    bool getTmplPic(QString &tmplPic);

    // 检查模板是否为封面
    static bool isCover(const QVariantMap &data)
    {
        return (1 == data["pagetype"].toInt());
    }

    // 获取模板大小
    static QSize getSize(const QVariantMap &data)
    {
        QVariantMap size = data["size"].toMap();
        return QSize(size["width"].toInt(), size["height"].toInt());
    }

    //static const QVariantList &getLayers(const QVariantMap &data){return data["layers"].toList();}

    const QVariantMap &getFrame(const QString &lid, QVariantMap &frame);

    // 获取模板相位
    uchar getLocations(void) const;

    // 获取模板相位
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

    static const QString &args2(QString &args, const QString &arg1, const QString &arg2)
    {
        args = tr("\"%1\" \"%2\"").arg(arg1).arg(arg2);
        return args;
    }

    static void useZip(QProcess &tmaker,
                       ZipUsage usage,
                       const QString &arguments,
                       bool block = false);

    static bool moveTo(QString &fileName, QString dirName, bool overwrite = true);

    static bool deleteDir(const QString &dir, bool all = true);

    static void parseTest(const QVariantMap &data);

    void remove(void){remove(getId());}

#if LOAD_FROM_MEMORY
    const QVariantMap &loadPictures(void);

    void inportPictures(const QString &tmplFile, const QVariantMap &pictures)
    {
        m_tmplFile = tmplFile;
        m_pictures = pictures;
    }

    const QVariantMap &getPictures(void){return m_pictures;}
#else
    const PicturesMap &loadPictures(void);

    const PicturesMap &getPictures(void){return m_pictures;}
#endif

protected slots:
    void onAccept(const QVariantMap &belongings);

private slots:
    void processFinished(int, QProcess::ExitStatus);

private:
    // 加载模板图片
    void loadPicture(QVariantMap &data, QString tmplPic = QString());

    // 获取模板ID
    int getId(void);

    // 移除模板
    void remove(int tid);

    QProcess m_tmaker;
    QString m_tmplFile, m_tmplPic, m_currFile;

#if LOAD_FROM_MEMORY
    QVariantMap m_pictures;
#else
    PicturesMap m_pictures;
#endif

    TemplateEngine::SqlThread m_sql;
    friend class TemplateEngine::SqlThread;
};



#endif // TEMPLATECHILDWIDGET_H
