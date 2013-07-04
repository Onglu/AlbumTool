#include "TemplateChildWidget.h"
#include "parser/json.h"
#include "parser/FileParser.h"
#include "page/TaskPageWidget.h"
#include "defines.h"
#include <QDebug>
#include <QMessageBox>
#include <QtSql>
#include <QTime>

using namespace QtJson;
using namespace TemplatesSql;

TemplateChildWidget::TemplateChildWidget(int index,
                                         const QString &file,
                                         int usedTimes,
                                         TaskPageWidget *parent) :
    PictureChildWidget(file, QSize(90, 130), true, parent),
    m_tmplFile(QDir::toNativeSeparators(file)),
    m_sql(this)
{
    setIndexLabel(index, NULL, QPoint(9, 3));

    m_records.insert("template_file", m_tmplFile);
    m_records.insert("used_times", usedTimes);

    if (QFile::exists(m_tmplFile))
    {
        connect(&m_sql, SIGNAL(finished()), &m_sql, SLOT(quit()));
        connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));
        useZip(m_tmaker, ZipUsageRead, m_tmplFile + " page.dat");
    }
    else
    {
        remove();
        setPictureLabel(QPixmap(m_tmplPic), QSize(72, 100), DRAGGABLE_TEMPLATE, this, QPoint(9, 21));
        QVariantMap &belongings = m_picLabel->getBelongings();
        belongings["template_file"] = m_tmplFile;
        belongings["picture_file"] = m_tmplPic;
        belongings["used_times"] = usedTimes;
    }
}

TemplateChildWidget::TemplateChildWidget(const QString &file, DraggableLabel *label) :
    PictureChildWidget(file),
    m_tmplFile(file),
    m_sql(this)
{
    m_picLabel = label;
    connect(&m_sql, SIGNAL(finished()), &m_sql, SLOT(quit()));
    connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));
    useZip(m_tmaker, ZipUsageRead, m_tmplFile + " page.dat");
}

const QVariantMap &TemplateChildWidget::getChanges(void)
{
    QVariantMap belongings = m_picLabel->getBelongings();
    m_records["template_file"] = belongings["template_file"];
    m_records["used_times"] = belongings["used_times"];
    //qDebug() << __FILE__ << __LINE__ << m_index << belongings["used_times"].toString() << m_picLabel->getPictureFile();
    return PictureChildWidget::getChanges();
}

bool TemplateChildWidget::getTmplPic(QString &tmplPic)
{
    QString fileName = m_tmplFile;
    int pos = fileName.lastIndexOf(PKG_FMT, -1, Qt::CaseInsensitive);

    if (-1 != pos)
    {
        tmplPic = fileName.replace(pos, strlen(PKG_FMT), PIC_FMT);
        if (QFile::exists(tmplPic))
        {
            return true;
        }
    }

    return false;
}

int TemplateChildWidget::getId()
{
    if (m_tmplPic.isEmpty())
    {
        getTmplPic(m_tmplPic);
    }

    QString sql = tr("select id from template where fileurl='%1' and page_id='%2'").arg(m_tmplPic).arg(m_container->getPageId());
    return SqlHelper::getId(sql);
}

void TemplateChildWidget::onAccept(const QVariantMap &belongings)
{
    //qDebug() << __FILE__ << __LINE__ << belongings["template_file"].toString() << belongings["used_times"].toInt();

    QString tmplFile = belongings["template_file"].toString();
    if (!tmplFile.isEmpty() && m_tmplFile != tmplFile)
    {
        m_tmplFile = tmplFile;
    }

    QString tmplPic = belongings["picture_file"].toString();
    if (!tmplPic.isEmpty() && m_tmplPic != tmplPic)
    {
        m_tmplPic = tmplPic;
    }

    m_records["used_times"] = belongings["used_times"];

    if (!m_pictures.isEmpty())
    {
        m_pictures.clear();
    }

    PictureChildWidget::onAccept(belongings);
}

void TemplateChildWidget::useZip(QProcess &tmaker,
                                 ZipUsage usage,
                                 const QString &arguments,
                                 bool block)
{
    if (arguments.isEmpty())
    {
        return;
    }

    QString program(MAKER_NAME);

    switch (usage)
    {
    case ZipUsageCompress:
        program += QString(" -c ");
        break;
    case ZipUsageAppend:
        program += QString(" -a ");
        break;
    case ZipUsageRemove:
        program += QString(" -x ");
        break;
    case ZipUsageUncompress:
        program += QString(" -u ");
        break;
    case ZipUsageList:
        program += QString(" -l ");
        break;
    case ZipUsageRead:
        program += QString(" -r ");
        break;
    case ZipUsageEncrypt:
        program += QString(" -e ");
        break;
    case ZipUsageDecrypt:
        program += QString(" -d ");
        break;
    default:
        return;
    }

    program += arguments;
    //qDebug() << __FILE__ << __LINE__ << program;

    if (!block)
    {
        tmaker.start(program);
        tmaker.waitForFinished();
    }
    else
    {
        tmaker.execute(program);
    }
}

void TemplateChildWidget::processFinished(int ret, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(ret);

#if PKG_ENCRYPT
    //qDebug() << __FILE__ << __LINE__ << &m_parser << m_parser.isFinished() << m_pkgFile << m_parser.getPkgFile();
    if (m_parser.isFinished())
    {
        return;
    }
#endif

    if (QProcess::CrashExit == exitStatus)
    {
        QMessageBox::critical(m_container, tr("解析失败"), tr("错误信息：tmaker.exe has crashed!"), tr("确定"));
        return;
    }

    static QVariantMap results;
    QByteArray out = m_tmaker.readAllStandardOutput();
    QString content(out);
    m_tmaker.close();

    if (content.startsWith("compress:"))
    {
//            moveTo(m_psdPic, m_tmpDir);
//            deleteDir(content.mid(9));
    }

    if (content.startsWith("data:"))
    {
        bool ok;
        QString data = content.mid(5);
        //qDebug() << __FILE__ << __LINE__ << m_tmplFile;
        results = QtJson::parse(data, ok).toMap();
        if (data.isEmpty() || !ok)
        {
            QMessageBox::warning(this, tr("导入失败"), tr("导入的相册模板包格式无效！"), tr("确定"));
            return;
        }

        if (!getTmplPic(m_tmplPic))
        {
            m_currFile = m_tmplPic = results["name"].toString() + PIC_FMT;
            useZip(m_tmaker, ZipUsageRead, m_tmplFile + " " + m_tmplPic);
        }
        else
        {
            loadPicture(results);
        }
    }

    if (content.startsWith("picture:"))
    {
        QString name = content.mid(8);
        QPixmap pix(name);
        //qDebug() << __FILE__ << __LINE__ << m_currFile << pix.isNull();

        //QPixmap pixe;
        //if (pixe.loadFromData(m_pictures[m_currFile].toByteArray()))

        if (m_tmplPic == m_currFile)
        {
            loadPicture(results, name);
        }
        else
        {
            QFile file(name);
            if (!pix.isNull() && file.open(QIODevice::ReadOnly))
            {
                m_pictures.insert(m_currFile, file.readAll());
                file.remove();
            }
        }

        QFile::remove(m_currFile);
    }
}

bool SqlThread::existing(const QString &pageId)
{
    QSqlQuery query;
    QString sql = tr("select id from template where name='%1' and portrait_count=%2 and landscape_count=%3 and ver='%4' and fileurl='%5' and fileguid='%6' and page_type=%7 and page_id='%8'").arg(m_data["name"].toString()).arg(m_data["portraitCount"].toInt()).arg(m_data["landscapeCount"].toInt()).arg(m_data["ver"].toString()).arg(m_widget->m_tmplPic).arg(m_data["id"].toString()).arg(m_data["pagetype"].toInt()).arg(pageId);

    query.exec(sql);
    while (query.next())
    {
        return true;
    }

    return false;
}

void SqlThread::run()
{
    //QTime tm;
    //tm.start();

    SqlHelper sh;
    if (!sh.connectDb())
    {
        qDebug() << __FILE__ << __LINE__ << "error:" << sh.getError();
        goto end;
    }

    if (!m_search)
    {
        QString pageId = m_widget->m_container->getPageId();
        if (existing(pageId))
        {
            goto end;
        }

        QSqlQuery query;
        int tid = 0;
        QString sql = "insert into template(name,portrait_count,landscape_count,ver,fileurl,fileguid,page_type,page_id) ";
        sql += tr("values('%1',%2,%3,'%4','%5','%6',%7,'%8')").arg(m_data["name"].toString()).arg(m_data["portraitCount"].toInt()).arg(m_data["landscapeCount"].toInt()).arg(m_data["ver"].toString()).arg(m_widget->m_tmplPic).arg(m_data["id"].toString()).arg(m_data["pagetype"].toInt()).arg(pageId);
        query.exec(sql);
        tid = query.lastInsertId().toInt();
        //qDebug() << __FILE__ << __LINE__ << "inserts one record costs:" << tm.elapsed();

        QVariantList tags = m_data["tags"].toList();
        foreach (const QVariant &tag, tags)
        {
            QVariantMap property = tag.toMap();
            QString name = property["name"].toString();
            QString type = property["type"].toString();
            int pid = 0;

            sql = tr("select id from tproperty where ptype='%1' and name='%2'").arg(type).arg(name);
            query.exec(sql);
            while (query.next())
            {
                pid = query.value(0).toInt();
                sql = tr("insert into template_property values(%1,%2)").arg(tid).arg(pid);
                query.exec(sql);
            }

            if (!pid)
            {
                sql = tr("insert into tproperty(ptype,name) values('%1','%2')").arg(type).arg(name);
                query.exec(sql);
                //qDebug() << __FILE__ << __LINE__ << sql << query.lastError().databaseText();
                pid = query.lastInsertId().toInt();
                sql = tr("insert into template_property values(%1,%2)").arg(tid).arg(pid);
                query.exec(sql);
            }
        }
    }

end:
    //qDebug() << __FILE__ << __LINE__ << "inserts template properties costs:" << tm.elapsed();
    emit finished();
}

void TemplateChildWidget::remove(int tid)
{
    if (tid)
    {
        QSqlQuery query;
        QString sql = tr("delete from template where id='%1'").arg(tid);
        query.exec(sql);
        sql = tr("delete from template_property where templateid='%1'").arg(tid);
        query.exec(sql);
    }
}

bool TemplateChildWidget::moveTo(QString &fileName, QString dirName, bool overwrite)
{
    QFile file(fileName);
    if (!file.exists() || fileName.startsWith(dirName, Qt::CaseInsensitive))
    {
        return false;
    }

    QDir dir(dirName);
    if (!dir.exists() && !dir.mkpath(dirName))
    {
        return false;
    }

    QChar sep = QDir::separator();
    QString tl(sep), name = fileName.right(fileName.length() - fileName.lastIndexOf(sep) - 1);
    if (tl != dirName.right(1))
    {
        dirName = QDir::toNativeSeparators(dirName) + tl;
    }

    if (dir.exists(name))
    {
        if (!overwrite)
        {
            return false;
        }
        else
        {
            dir.remove(name);
        }
    }

    name = dirName + name;
    if (file.copy(name))
    {
        file.remove();
        fileName = name;
    }

    return true;
}

void TemplateChildWidget::parseTest(const QVariantMap &data)
{
    QVariantMap::const_iterator iter = data.constBegin();
    while (iter != data.constEnd())
    {
        if ("size" == iter.key())
        {
            qDebug() << __FILE__ << __LINE__ << iter.key() << ":" << iter.value().toMap();
        }
        else if ("tags" == iter.key())
        {
            qDebug() << __FILE__ << __LINE__ << iter.key() << ":" << iter.value().toList();
        }
        else if ("layers" == iter.key())
        {
            qDebug() << __FILE__ << __LINE__ << iter.key() << ":" << iter.value().toList();
        }
//        else if ("landscapeCount" == iter.key())
//        {
//            m_locations[0] = (uchar)iter.value().toUInt();
//        }
//        else if ("portraitCount" == iter.key())
//        {
//            m_locations[1] = (uchar)iter.value().toUInt();
//        }
//        else if ("name" == iter.key())
//        {
//            m_currFile = m_tmplPic = QString("%1.psd.png").arg(iter.value().toString());
//        }
//        else
//        {
//            m_bases.insert(iter.key(), iter.value());
//        }

        ++iter;
    }
}

void TemplateChildWidget::loadPicture(QVariantMap &data, QString tmplPic)
{
    if (m_tmplPic.isEmpty())
    {
        return;
    }

    if (!tmplPic.isEmpty() && QFile::exists(tmplPic))   // get the preview picture
    {
        QString tmplDir = m_tmplFile.left(m_tmplFile.lastIndexOf(QDir::separator()) + 1);
        moveTo(tmplPic, tmplDir);
        getTmplPic(m_tmplPic);
        QFile::rename(tmplPic, m_tmplPic);
    }

    bool copiedLabel = true;
    if (!m_picLabel)
    {
        setPictureLabel(QPixmap(m_tmplPic), QSize(72, 100), DRAGGABLE_TEMPLATE, this, QPoint(9, 21));
        copiedLabel = false;
    }

    int usedTimes = m_records["used_times"].toInt();
    QVariantMap &belongings = m_picLabel->getBelongings();
    belongings["template_file"] = m_tmplFile;
    belongings["picture_file"] = m_tmplPic;
    belongings["used_times"] = usedTimes;
    belongings["page_data"] = data;

    if (!copiedLabel)
    {
        if (usedTimes)
        {
            //qDebug() << __FILE__ << __LINE__ << usedTimes << m_tmplFile;
            m_picLabel->accept(true);
        }

        m_sql.bindData(data);
        m_sql.start();
    }

    data.clear();
}

const QVariantMap &TemplateChildWidget::loadPictures()
{
    Q_ASSERT(m_picLabel);

    if (!m_pictures.isEmpty())
    {
        return m_pictures;
    }

    QVariantMap belongings = m_picLabel->getBelongings();
    QVariantMap data = belongings["page_data"].toMap();
    QVariantList layers = data["layers"].toList();

    QTime tm;
    tm.start();

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        m_currFile = data["id"].toString();
        if (LT_Photo == data["type"].toInt())
        {
            m_currFile += PIC_FMT;
            //continue;
        }
        else
        {
            m_currFile += ".png";
        }

        useZip(m_tmaker, ZipUsageRead, m_tmplFile + " " + m_currFile);
        //qDebug() << __FILE__ << __LINE__ << m_currFile;
    }

    qDebug() << __FILE__ << __LINE__ << "costs" << tm.elapsed() << "ms after loaded" << m_pictures.size() << "pictures";

    return m_pictures;
}
