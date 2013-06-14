#include "FileParser.h"
#include "parser/json.h"
#include <QFileDialog>
#include <QMessageBox>

#define MAGIC_NUMBER    0xBCC2AEFD
#define VERSION_NUMBER  123

using namespace QtJson;

int FileParser::importFiles(const QString &dirKey,
                            const QString &caption,
                            const QString &filter,
                            QStringList &fileNames,
                            bool multiSel)
{
    Q_ASSERT(m_parent);

    m_Settings.beginGroup("Location");

    QString dirName = m_Settings.value(dirKey).toString();
    if (dirName.isEmpty() || !QDir(dirName).exists())
    {
        dirName = QDir::homePath() + "/Pictures/";
    }

    if (multiSel)
    {
        fileNames = QFileDialog::getOpenFileNames(m_parent, caption, dirName, filter);
    }
    else
    {
        QString fileName = QFileDialog::getOpenFileName(m_parent, caption, dirName /*+ fileNames.first()*/, filter);
        if (!fileName.isEmpty())
        {
            fileNames.clear();
            fileNames << fileName;
        }
    }

    int count = 0;
    if ((count = fileNames.size()))
    {
        QString fileName = QDir::toNativeSeparators(fileNames.first());
        dirName = fileName.left(fileName.lastIndexOf(QDir::separator()) + 1);
        m_Settings.setValue(dirKey, dirName);
    }

    m_Settings.endGroup();

    return count;
}

void FileParser::printList(const QVariantMap &vm, const QString &section)
{
    QVariantList results = vm[section].toList();

    foreach (QVariant result, results)
    {
        int index = 0;
        QVariantMap records = result.toMap();
        QVariantMap::const_iterator iter = records.constBegin();

        while (index < records.size())
        {
            if ("photos_list" == iter.key())
            {
                QVariantList photos = records["photos_list"].toList();
                foreach (const QVariant &photo, photos)
                {
                    qDebug() << section << "> photo is " << photo.toString();
                }
            }
            else if ("template_attribute" == iter.key())
            {
                QString tmpl = iter.value().toString();
                QString file = tmpl.left(tmpl.lastIndexOf('|') - 1);
                QString locations = tmpl.right(tmpl.length() - file.length() - 2);
                qDebug() << section << "> template is " << "file " << file << "locations" << locations;
            }
            else
            {
                QString key = iter.key();
                if (key.startsWith("search_"))
                {
                    qDebug() << section << "> search condition" << iter.key() << ":" << iter.value().toString();
                }
                else
                {
                    qDebug() << section << ">" << iter.key() << ":" << iter.value().toString();
                }
            }

            index++;
            ++iter;
        }
    }
}

bool FileParser::openTask(QVariantList &photos, QVariantList &templates, QVariantList &albums)
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(m_parent, tr("打开失败"), tr("请检查目标文件是否存在！"), tr("确定"));
        return false;
    }

    QDataStream in(&file);

    // Read and check the header
    quint32 magic;
    in >> magic;
    if (magic != MAGIC_NUMBER)
    {
        QMessageBox::critical(m_parent, tr("打开失败"), tr("文件格式无效！"), tr("确定"));
        return false;
    }

    // Read the version
    qint32 version;
    in >> version;
    if (version != VERSION_NUMBER)
    {
        QMessageBox::critical(m_parent, tr("打开失败"), tr("文件版本号错误！"), tr("确定"));
        return false;
    }

    in.setVersion(QDataStream::Qt_4_8);

    QVariantMap xcrw;
    in >> xcrw;
    //qDebug() << __FILE__ << __LINE__ << xcrw;

    QVariantMap::const_iterator iter = xcrw.constBegin();
    while (iter != xcrw.constEnd())
    {
        if ("photos" == iter.key())
        {
            photos = xcrw["photos"].toList();
        }
        else if ("templates" == iter.key())
        {
            templates = xcrw["templates"].toList();
        }
        else if ("albums" == iter.key())
        {
            albums = xcrw["albums"].toList();
        }

        ++iter;
    }

    return true;
}

void FileParser::saveTask()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    QDataStream out(&file);

    // Write a header with a "magic number" and a version
    out << (quint32)MAGIC_NUMBER;
    out << (qint32)VERSION_NUMBER;

    out.setVersion(QDataStream::Qt_4_8);

    QVariantMap xcrw;
    QVariantList photos, templates, albums;

    QVariantMap photo;
    photos << photo;
    xcrw.insert("photos", photos);

    QVariantMap tmpl;
    templates << tmpl;
    xcrw.insert("templates", templates);

    QVariantMap album;
    albums << album;
    xcrw.insert("albums", albums);

    out << xcrw;
}

void FileParser::saveTask(const QVariantList &photos,
                          const QVariantList &templates,
                          const QVariantList &albums)
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    QDataStream out(/*this*/&file);

    // Write a header with a "magic number" and a version
    out << (quint32)MAGIC_NUMBER;
    out << (qint32)VERSION_NUMBER;

    out.setVersion(QDataStream::Qt_4_8);

    QVariantMap xcrw;
    xcrw.insert("photos", photos);
    xcrw.insert("templates", templates);
    xcrw.insert("albums", albums);
    out << xcrw;
}

bool FileParser::openTemplate(QVariantMap &bases, QVariantMap &size, QVariantList &tags, QVariantList &layers)
{
    bool ok = false;

    QFile jf(m_fileName);
    if (!jf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ok;
    }

    QByteArray data = jf.readAll();
    if (data.isEmpty())
    {
        return ok;
    }

    QVariantMap tmpl = QtJson::parse(QString(data), ok).toMap();
    if (!ok)
    {
        return ok;
    }

    QVariantMap::const_iterator iter = tmpl.constBegin();
    while (iter != tmpl.constEnd())
    {
        if ("size" == iter.key())
        {
            size = iter.value().toMap();
        }
        else if ("tags" == iter.key())
        {
            tags = iter.value().toList();
        }
        else if ("layers" == iter.key())
        {
            layers = iter.value().toList();
        }
        else
        {
            bases.insert(iter.key(), iter.value());
        }

        ++iter;
    }

    return ok;
}
