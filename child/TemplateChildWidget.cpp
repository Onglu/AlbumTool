#include "TemplateChildWidget.h"
#include "parser/json.h"
#include "parser/FileParser.h"
#include "page/TaskPageWidget.h"
#include "defines.h"
//#include "SqlEngine.h"
#include <QDebug>
#include <QMessageBox>
#include <QtSql>
#include <QTime>

using namespace QtJson;
using namespace TemplatesSql;

TemplateChildWidget::TemplateChildWidget(int index,
                                         const QString &tmplFile,
                                         int usedTimes,
                                         TaskPageWidget *parent) :
    PictureChildWidget(QSize(90, 130), true, parent),
    m_tmplFile(QDir::toNativeSeparators(tmplFile)),
    m_sql(this)
{
    setToolTip(tr("鼠标点中以拖放"));

    setIndexLabel(index, NULL, QPoint(9, 3));

    m_records.insert("template_file", m_tmplFile);
    m_records.insert("used_times", usedTimes);

    connect(&m_sql, SIGNAL(finished()), &m_sql, SLOT(quit()));
    connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));

    useZip(m_tmaker, ZipUsageRead, m_tmplFile + " page.dat");
}

void TemplateChildWidget::setTemplate(DraggableLabel &label,
                                      const QString &tmplPic,
                                      const QString &tmplFile,
                                      int cover,
                                      int used_times)
{
#ifndef FROM_PACKAGE
    QVariantMap belongings;
    belongings["picture_file"] = tmplPic;
    belongings["template_file"] = tmplFile;
    belongings["page_type"] = cover;
    belongings["used_times"] = used_times;
    label.setBelongings(belongings);
#else

#endif
}

const QVariantMap &TemplateChildWidget::getChanges(void)
{
    QVariantMap belongings = m_picLabel->getBelongings();
    m_tmplFile = belongings["template_file"].toString();
    m_records["template_file"] = belongings["template_file"];
    m_records["used_times"] = belongings["used_times"];
    //qDebug() << __FILE__ << __LINE__ << m_index << belongings["used_times"].toString() << m_picLabel->getPictureFile();
    return PictureChildWidget::getChanges();
}

bool TemplateChildWidget::getTmplFile(QString &tmplFile, bool pkgFile)
{
    if (pkgFile)
    {
        tmplFile = m_tmplFile;
        return true;
    }

    QString fileName = m_tmplFile;
    int pos = fileName.lastIndexOf(PKG_FMT, -1, Qt::CaseInsensitive);
    if (-1 != pos)
    {
        tmplFile = fileName.replace(pos, strlen(PKG_FMT), ".png");
        if (QFile::exists(tmplFile))
        {
            return true;
        }
    }

    return false;
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

//    if (ZipUsageAppend == m_usage /*&& !m_tmpFile.isEmpty()*/)
//    {
//        //qDebug() << __FILE__ << __LINE__ << m_tmpFile;
//        //access();
//    }

    if (content.startsWith("data:"))
    {
        bool ok;
        QString data = content.mid(5);
        //qDebug() << __FILE__ << __LINE__ << data;
        results = QtJson::parse(data, ok).toMap();
        if (data.isEmpty() || !ok)
        {
            QMessageBox::warning(this, tr("导入失败"), tr("导入的相册模板包格式无效！"), tr("确定"));
            return;
        }

        if (!getTmplFile(m_tmplPic, false))
        {
            m_currFile = m_tmplPic = results["name"].toString() + ".psd.png";
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
        qDebug() << __FILE__ << __LINE__ << m_currFile << pix.isNull();

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

bool SqlThread::existing()
{
    QSqlQuery query;
    QString sql = tr("select id from template where name='%1' and portrait_count=%2 and landscape_count=%3 and ver='%4' and fileurl='%5' and fileguid='%6' and page_type=%7").arg(m_data["name"].toString()).arg(m_data["portraitCount"].toInt()).arg(m_data["landscapeCount"].toInt()).arg(m_data["ver"].toString()).arg(m_widget->m_tmplPic).arg(m_data["id"].toString()).arg(m_data["pagetype"].toInt());
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
        if (existing())
        {
            goto end;
            //m_widget->remove(tid);
            //qDebug() << __FILE__ << __LINE__ << "time costs:" << tm.elapsed();
        }

        QSqlQuery query;
        int tid = 0, pid = 0;
        QString sql = "insert into template(name,portrait_count,landscape_count,ver,fileurl,fileguid,page_type) ";
        sql += tr("values('%1',%2,%3,'%4','%5','%6',%7)").arg(m_data["name"].toString()).arg(m_data["portraitCount"].toInt()).arg(m_data["landscapeCount"].toInt()).arg(m_data["ver"].toString()).arg(m_widget->m_tmplPic).arg(m_data["id"].toString()).arg(m_data["pagetype"].toInt());
        query.exec(sql);
        tid = query.lastInsertId().toInt();
        //qDebug() << __FILE__ << __LINE__ << "inserts one record costs:" << tm.elapsed();

        QVariantList tags = m_data["tags"].toList();
        foreach (const QVariant &tag, tags)
        {
            QVariantMap property = tag.toMap();
            sql = tr("select id from tproperty where ptype='%1' and name='%2'").arg(property["type"].toString()).arg(property["name"].toString());
            query.exec(sql);
            while (query.next())
            {
                pid = query.value(0).toInt();
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

//void TemplateChildWidget::parse()
//{
//    QVariantMap::const_iterator iter = m_belongings.constBegin();
//    while (iter != m_belongings.constEnd())
//    {
//        if ("size" == iter.key())
//        {
//            m_size = iter.value().toMap();
//        }
//        else if ("tags" == iter.key())
//        {
//            m_tags = iter.value().toList();
//        }
//        else if ("layers" == iter.key())
//        {
//            m_layers = iter.value().toList();
//        }
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

//        ++iter;
//    }
//}

void TemplateChildWidget::loadPicture(QVariantMap &data, QString tmplPic)
{
    if (m_tmplPic.isEmpty())
    {
        return;
    }

    if (!tmplPic.isEmpty())
    {
        QString tmplDir = m_tmplFile.left(m_tmplFile.lastIndexOf(QDir::separator()) + 1);
        moveTo(tmplPic, tmplDir);
        getTmplFile(m_tmplPic, false);
        QFile::rename(tmplPic, m_tmplPic);
    }

    setPictureLabel(QPixmap(m_tmplPic), QSize(72, 100), DRAGGABLE_TEMPLATE, this, QPoint(9, 21));
    QVariantMap &belongings = m_picLabel->getBelongings();
    belongings["template_file"] = m_tmplFile;
    belongings["picture_file"] = m_tmplPic;
    belongings["used_times"] = m_records["used_times"];
    belongings["page_data"] = data;

    m_sql.bindData(data);
    m_sql.start();

    data.clear();
}

void TemplateChildWidget::loadPictures()
{
    Q_ASSERT(m_picLabel);

    QVariantMap belongings = m_picLabel->getBelongings();
    QVariantList layers = belongings["layers"].toList();

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        m_currFile = data["id"].toString() + ".png";
        useZip(m_tmaker, ZipUsageRead, m_tmplFile + " " + m_currFile);
        //qDebug() << __FILE__ << __LINE__ << filename;
    }
}

bool TemplateChildWidget::match(QVariantMap cond)
{
    if (m_data["pagetype"].toInt() != cond["pagetype"].toInt())
    {
        return false;
    }

    QVariantList tags = m_data["tags"].toList();
    int n = 1;
    bool sensitive = false;
    QString style, text = cond["风格"].toString();

    if (2 == cond.size() && text.isEmpty())
    {
        return true;
    }

    foreach (const QVariant &tag, tags)
    {
        QVariantMap data = tag.toMap();
        QString type = data["type"].toString();

        if ("风格" == type)
        {
            style = data["name"].toString();
        }

        if (!style.isEmpty() && !text.isEmpty())
        {
            sensitive = style.contains(text, Qt::CaseInsensitive) || text.contains(style, Qt::CaseInsensitive);
            //qDebug() << __FILE__ << __LINE__ << style << text << sensitive;
        }

        QVariantMap::iterator iter = cond.begin();
        while (iter != cond.end())
        {
            qDebug() << __FILE__ << __LINE__ << data["name"].toString() << type << iter.key() << ":" << iter.value().toString() << m_tmplFile;

            if (sensitive || (data["name"].toString() == iter.key() && type == iter.value().toString()))
            {
                //qDebug() << __FILE__ << __LINE__ << iter.key() << ":" << iter.value().toString() << m_tmplFile;
                qDebug() << __FILE__ << __LINE__ <<sensitive << style << text;
                cond.erase(iter);
                n++;
                break;
            }

            ++iter;
        }
    }

    if (!style.isEmpty() && !text.isEmpty() && !sensitive)
    {
        n = 0;
    }

    qDebug() << __FILE__ << __LINE__ <<sensitive << style << text << sensitive << n;

    return (1 < n);
}
