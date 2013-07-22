#include "FileParser.h"
#include "parser/json.h"
#include <QFileDialog>
#include <QMessageBox>

#define MAGIC_NUMBER    0xBCC2AEFD
#define VERSION_NUMBER  123

using namespace QtJson;

bool FileParser::openTask(QVariantList &photos, QVariantList &templates, QVariantList &albums)
{
    if (!openFile(QIODevice::ReadOnly))
    {
        QMessageBox::critical(m_parent, tr("打开失败"), tr("请检查文件是否存在！"), tr("确定"));
        return false;
    }

    QDataStream in(this);
    quint32 magic, version;

    try
    {
        // Read and check the header
        in >> magic;
        if (magic != MAGIC_NUMBER)
        {
            throw __LINE__;
        }

        // Read the version
        in >> version;
        if (version != VERSION_NUMBER)
        {
            throw __LINE__;
        }

        in.setVersion(QDataStream::Qt_4_8);
    }
    catch (int err)
    {
        QMessageBox::critical(m_parent, tr("打开失败"), tr("任务文件格式无效！\n错误码：%1").arg(err), tr("确定"));
        return false;
    }

    QVariantMap xcrw;
    in >> xcrw;
    //qDebug() << __FILE__ << __LINE__ << xcrw;

    QVariantMap::const_iterator iter = xcrw.constBegin();
    while (iter != xcrw.constEnd())
    {
        if ("id" == iter.key())
        {
            m_pageId = xcrw["id"].toString();
        }
        else if ("photos" == iter.key())
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
    if (!openFile(QIODevice::WriteOnly, false))
    {
        return;
    }

    QDataStream out(this);

    // Write a header with a "magic number" and a version
    out << (quint32)MAGIC_NUMBER;
    out << (qint32)VERSION_NUMBER;
    out.setVersion(QDataStream::Qt_4_8);

    QVariantMap xcrw;
    QVariantList photos, templates, albums;

    m_pageId = QUuid::createUuid().toString().mid(1, 36);
    xcrw.insert("id", m_pageId);

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
    if (!openFile(QIODevice::WriteOnly, false))
    {
        return;
    }

    QDataStream out(this);

    // Write a header with a "magic number" and a version
    out << (quint32)MAGIC_NUMBER;
    out << (qint32)VERSION_NUMBER;

    out.setVersion(QDataStream::Qt_4_8);

    QVariantMap xcrw;
    xcrw.insert("id", m_pageId);
    xcrw.insert("photos", photos);
    xcrw.insert("templates", templates);
    xcrw.insert("albums", albums);
    out << xcrw;
}

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

const QString &FileParser::getFileMd5(const QString &filePath, QString &md5)
{
    QFile localFile(filePath);

    if (!localFile.open(QFile::ReadOnly))
    {
        return md5;
    }

    QCryptographicHash ch(QCryptographicHash::Md5);
    quint64 totalBytes = 0;
    quint64 bytesWritten = 0;
    quint64 bytesToWrite = 0;
    quint64 loadSize = 1024 * 4;
    QByteArray buf;

    totalBytes = localFile.size();
    bytesToWrite = totalBytes;

    while (1)
    {
        if (bytesToWrite > 0)
        {
            buf = localFile.read(qMin(bytesToWrite, loadSize));
            ch.addData(buf);
            bytesWritten += buf.length();
            bytesToWrite -= buf.length();
            buf.resize(0);
        }
        else
        {
            break;
        }

        if (bytesWritten == totalBytes)
        {
            break;
        }
    }

    localFile.close();

    md5.append(ch.result().toHex());

    return md5;
}
