#include "TemplateChildWidget.h"
#include "parser/json.h"
#include "parser/FileParser.h"
#include "page/TaskPageWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QtSql>
#include <QTime>

using namespace QtJson;
using namespace TemplateEngine;

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

        QString args;
        useZip(m_tmaker, ZipUsageRead, args2(args, m_tmplFile, PAGE_DATA));
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

TemplateChildWidget::TemplateChildWidget(const QString &file, DraggableLabel *label, TaskPageWidget *parent) :
    PictureChildWidget(file, parent),
    m_tmplFile(file),
    m_sql(this)
{
    m_picLabel = label;
    connect(&m_sql, SIGNAL(finished()), &m_sql, SLOT(quit()));
    connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));

    if (QFile::exists(m_tmplFile))
    {
        QString args;
        useZip(m_tmaker, ZipUsageRead, args2(args, m_tmplFile, PAGE_DATA));
    }
}

const QVariantMap &TemplateChildWidget::getChanges(void)
{
    QVariantMap belongings = m_picLabel->getBelongings();
    m_records["template_file"] = belongings["template_file"];
    m_records["used_times"] = belongings["used_times"];
    return PictureChildWidget::getChanges();
}

bool TemplateChildWidget::getTmplPic(QString &tmplPic)
{
    QString fileName = m_tmplFile;
    int pos = fileName.lastIndexOf(PKG_FMT, -1, Qt::CaseInsensitive);

    if (-1 != pos)
    {
        tmplPic = fileName.replace(pos, strlen(PKG_FMT), ".png");
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
    //qDebug() << __FILE__ << __LINE__ << sql;
    return SqlHelper::getId(sql);
}

uchar TemplateChildWidget::getLocations() const
{
    QVariantMap belongings = m_picLabel->getBelongings();
    QVariantMap data = belongings["page_data"].toMap();
    return (data["portraitCount"].toInt() + data["landscapeCount"].toInt());
}

void TemplateChildWidget::onAccept(const QVariantMap &belongings)
{
    QString tmplFile = belongings["template_file"].toString();
    //qDebug() << __FILE__ << __LINE__ << tmplFile << m_tmplFile << m_pictures.size();

    if (!tmplFile.isEmpty() && m_tmplFile != tmplFile)
    {
        TemplateChildWidget *tmpl = m_container->getTmplWidget(tmplFile);
        if (tmpl)
        {
            QVariantMap pictures = m_pictures;
            m_pictures = tmpl->getPictures();
            tmpl->inportPictures(m_tmplFile, pictures);
            m_tmplFile = tmplFile;
        }
    }

    m_tmplPic = belongings["picture_file"].toString();
    m_records["used_times"] = belongings["used_times"];

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

        // If not existing the preview picture, extracts it again from the template package
        if (!getTmplPic(m_tmplPic))
        {
            m_currFile = m_tmplPic = PIC_NAME;

            QString args;
            useZip(m_tmaker, ZipUsageRead, args2(args, m_tmplFile, m_tmplPic));
        }
        else
        {
            loadPicture(results);
        }
    }

    if (content.startsWith("picture:"))
    {
        QString name = content.mid(8);

        //qDebug() << __FILE__ << __LINE__ << m_currFile;

        //QPixmap pixe;
        //if (pixe.loadFromData(m_pictures[m_currFile].toByteArray()))

        if (m_tmplPic == m_currFile)
        {
            loadPicture(results, name);
        }
        else
        {     
            QPixmap pix(name);

#if LOAD_FROM_MEMORY
            QFile file(name);
            if (!pix.isNull() && file.open(QIODevice::ReadOnly))
            {
                m_pictures.insert(m_currFile, qCompress(file.readAll(), 5));
                file.remove();
            }
#else
            if (!pix.isNull())
            {
                m_pictures.insert(m_currFile, new QPixmap(pix));
            }
            QFile::remove(name);
#endif
        }

        QFile::remove(m_currFile);
    }
}

int SqlThread::existing(/*const QString &pageId*/) const
{
    QSqlQuery query;
//    QString sql = tr("select id from template where name='%1' and portrait_count=%2 and landscape_count=%3 and ver='%4' and fileurl='%5' and fileguid='%6' and page_type=%7 and page_id='%8'").arg(m_data["name"].toString()).arg(m_data["portraitCount"].toInt()).arg(m_data["landscapeCount"].toInt()).arg(m_data["ver"].toString()).arg(m_widget->m_tmplPic).arg(m_data["id"].toString()).arg(m_data["pagetype"].toInt()).arg(pageId);

    QString sql = tr("select id from template where fileurl='%1' and fileguid='%2' and page_type=%3 ").arg(m_widget->m_tmplPic).arg(m_data["id"].toString()).arg(m_data["pagetype"].toInt());

    query.exec(sql);
    while (query.next())
    {
        return query.value(0).toInt();
    }

    return 0;
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
        QSqlQuery query;
        QString sql;
        QString pageId = m_widget->m_container->getPageId();
        int tid = existing(/*pageId*/);

        if (0 < tid)
        {
            sql = QString("update template set page_id='%1' where id=%2").arg(pageId).arg(tid);
            query.exec(sql);
            goto end;
        }

        sql = "insert into template(name,portrait_count,landscape_count,ver,fileurl,fileguid,page_type,page_id) ";
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

bool TemplateChildWidget::deleteDir(const QString &dir, bool all)
{
    bool ok = true;
    QDir directory(dir);

    if (dir.isEmpty() || !directory.exists())
    {
        return ok;
    }

    QStringList files = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    QList<QString>::iterator f = files.begin();
    while (f != files.end())
    {
        QString filePath = QDir::convertSeparators(directory.path() + '/' + (*f));
        QFileInfo fi(filePath);

        if (fi.isFile() || fi.isSymLink())
        {
            QFile::setPermissions(filePath, QFile::WriteOwner);
            ok = QFile::remove(filePath);
        }
        else if (fi.isDir())
        {
            ok = deleteDir(filePath);
        }

        if (!ok)
        {
            break;
        }

        ++f;
    }

    if (all)
    {
        //qDebug() << __FILE__ << __LINE__ << directory.path() << QDir::toNativeSeparators(directory.path()) << directory.dirName();
        return directory.rmdir(QDir::toNativeSeparators(directory.path()));
    }

    return ok;
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

//        if (m_pictures.isEmpty())
//        {
//            m_tm.start();
//            m_loader.start(10);
//        }

        loadPictures();
    }

    data.clear();
}

#if 0
void TemplateChildWidget::loadPicture()
{
    QVariantMap belongings = m_picLabel->getBelongings();
    QVariantMap data = belongings["page_data"].toMap();
    QVariantList layers = data["layers"].toList();
    int count = layers.size();

    qDebug() << __FILE__ << __LINE__ << m_lid << count << m_tmplPic;
    if (m_lid < count)
    {
        QVariantMap layer = layers[m_lid].toMap();
        m_currFile = layer["id"].toString();
        if (LT_Photo == layer["type"].toInt())
        {
            return;
        }
        else
        {
            m_currFile += ".png";
        }

        QString args;
        useZip(m_tmaker, ZipUsageRead, args2(args, m_tmplFile, m_currFile));

        if (count <= ++m_lid)
        {
            qDebug() << __FILE__ << __LINE__ << "costs" << m_tm.elapsed() << "ms after loaded" << m_lid << "pictures";
            m_loader.stop();
            m_lid = 0;
        }
    }
}
#endif

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

    //QTime tm;
    //tm.start();

    //QCoreApplication::postEvent(m_container->getEditPage(), new QEvent(CustomEvent_Load_BEGIN));
    //QCoreApplication::postEvent(m_container, new QEvent(CustomEvent_Load_BEGIN));

    foreach (const QVariant &layer, layers)
    {
        QVariantMap data = layer.toMap();
        m_currFile = data["id"].toString();
        if (LT_Photo == data["type"].toInt())
        {
            //m_currFile += PIC_FMT;
            continue;
        }
        else
        {
            m_currFile += ".png";
        }

        QString args;
        useZip(m_tmaker, ZipUsageRead, args2(args, m_tmplFile, m_currFile));
        //qDebug() << __FILE__ << __LINE__ << m_currFile;
    }

    //QCoreApplication::postEvent(m_container->getEditPage(), new QEvent(CustomEvent_Load_Finished));

    //qDebug() << __FILE__ << __LINE__ << "loaded" << m_pictures.size() << "pictures from" << m_tmplFile << "costs" << tm.elapsed() << "ms in total";

    return m_pictures;
}

const QVariantMap &TemplateChildWidget::getFrame(const QString &lid, QVariantMap &frame)
{
    if (!lid.isEmpty())
    {
        QVariantMap belongings = m_picLabel->getBelongings();
        QVariantMap data = belongings["page_data"].toMap();
        QVariantList layers = data["layers"].toList();

        foreach (const QVariant &layer, layers)
        {
            QVariantMap data = layer.toMap();
            if (lid == data["id"].toString())
            {
                frame = data["frame"].toMap();
                break;
            }
        }
    }

    return frame;
}
