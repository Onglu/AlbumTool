#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QtCore>
#include <QFile>
#include <QSettings>

#define Task_New    1
#define Task_Open   2
#define Task_Save   3
#define Task_SaveAs 4

class FileLocker
{
public:
    FileLocker(void) : m_fp(NULL){}
    FileLocker(const QString &file) : m_file(file), m_fp(NULL){lock(file);}
    //~FileLocker(void){unlock();}

    FileLocker &operator=(const FileLocker &fl)
    {
        m_file = fl.m_file;
        m_fp = fl.m_fp;
        return *this;
    }

    void lock(const QString &file = QString())
    {
        if (m_file != file)
        {
            unlock();
            if (!file.isEmpty())
            {
                m_file = file;
            }
        }

        if (QFile::exists(m_file))
        {
            m_fp = fopen(m_file.toStdString().c_str(), "rb");
        }
    }

    void unlock()
    {
        if (m_fp)
        {
            fclose(m_fp);
            m_fp = NULL;
        }
    }

private:
    QString m_file;
    FILE *m_fp;
};

class FileParser: public QFile
{
public:
    explicit FileParser(QWidget *parent = 0) : QFile(NULL), m_parent(parent){}
    FileParser(const QString &fileName, QWidget *parent = 0) : QFile(fileName), m_fileName(fileName), m_parent(parent){}
    ~FileParser(void){m_locker.unlock();close();}

    bool openTask(QVariantList &photos, QVariantList &templates, QVariantList &albums);

    /* Save an empty(note: its contents is not empty) album task */
    void saveTask(void);

    void saveTask(const QVariantList &photos,
                  const QVariantList &templates,
                  const QVariantList &albums);

    void clear(void)    // Clear task file content
    {
        if (isValid())
        {
            open(QIODevice::WriteOnly);
        }
    }

    bool isValid(void)
    {
        if (!m_fileName.isEmpty() && exists() && size())
        {
            return true;    // This is a valid task file
        }

        return false;
    }

    void setParsingFile(const QString &fileName)
    {
        m_locker.unlock();
        setFileName(m_fileName = fileName);
        m_locker.lock(m_fileName);
    }

    QString getParsingFile(void) const {return m_fileName;}

    const QString &getPageId(void) const {return m_pageId;}

    /* File name format: "C:/path1/file.txt" */
    int importFiles(const QString &dirKey,
                    const QString &caption,
                    const QString &filter,
                    QStringList &fileNames,
                    bool multiSel = true);

    static const QString &getFileMd5(const QString &filePath, QString &md5);

private:
    QWidget *m_parent;
    QSettings m_Settings;
    FileLocker m_locker;
    QString m_fileName, m_pageId;
};

#endif // FILEPARSER_H
