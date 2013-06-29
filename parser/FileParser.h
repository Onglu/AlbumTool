#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QtCore>
#include <QFile>
#include <QSettings>

#define Task_New    1
#define Task_Open   2
#define Task_Save   3
#define Task_SaveAs 4

class FileParser: public QFile
{
public:
    explicit FileParser(QWidget *parent = 0) : QFile(NULL), m_parent(parent){}
    FileParser(const QString &fileName, QWidget *parent = 0) : QFile(fileName), m_fileName(fileName), m_parent(parent){}

//    FileParser &operator=(const FileParser &fp)
//    {
//        m_taskFile = fp.getTaskFile();
//    }

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

    void setParsingFile(const QString &fileName){setFileName(m_fileName = fileName);}
    QString getParsingFile(void) const {return m_fileName;}

    const QString &getPageId(void) const {return m_pageId;}

    /* File name format: "C:/path1/file.txt" */
    int importFiles(const QString &dirKey,
                    const QString &caption,
                    const QString &filter,
                    QStringList &fileNames,
                    bool multiSel = true);

private:
    QWidget *m_parent;
    QSettings m_Settings;
    QString m_fileName, m_pageId;
};

#endif // FILEPARSER_H
