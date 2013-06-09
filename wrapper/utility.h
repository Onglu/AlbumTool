#ifndef UTILITY_H
#define UTILITY_H

#include <QVector>
#include <QStringList>
#include <QVariant>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class Converter
{
public:
    Converter();

    static const QStringList &v2s(const QVector<QString> strVector, QStringList &strList);
    static int num(const QStringList &strList, bool empty = false/* equal with its size */);
    static qreal rotation(qreal &current, qreal degree)
    {
        if (degree)
        {
            current += degree;
        }
        else
        {
            current = 0;
        }

        if (360.0f <= current || -360.0f >= current)
        {
            current = 0;
        }

        return current;
    }
};

enum ViewType{ViewType_Photo, ViewType_Template, ViewType_Album};

class LoaderThread : public QThread
{
    Q_OBJECT
public:
    LoaderThread(ViewType view) : m_running(false), m_suspended(false), m_viewType(view){}
    LoaderThread(const LoaderThread &loader) : m_running(false), m_suspended(false), m_viewType(loader.m_viewType){}
    ~LoaderThread(void){end();}

    void loadList(const QVariantList &recordsList){m_recordsList = recordsList;}
    void loadList(const QStringList &existingsList, const QStringList &filesList)
    {
        m_existingsList = existingsList;
        m_filesList = filesList;
    }

    void begin(void)
    {
        if (!isRunning())
        {
            m_running = true;
            start();
        }
        else
        {
            reset();    // resume it
        }
    }

    void end(void);

    bool isActive(void)
    {
        if (m_running)
        {
            if (m_suspended)
            {
                return false;
            }

            return true;
        }
        else
        {
            return false;
        }
    }

signals:
    void itemAdded(int index, const QString &file, qreal angle, Qt::Axis axis, int usedTimes);
    void itemAdded(int index, const QString &file, int usedTimes);
    void itemAdded(int index, const QStringList &filesList, const QString &file/* Template data file */);
    void loadFinished(int from/* 0: load from the task file, 1: load from the local disk */);

protected:
    void run();

private:
    void reset(void);

    bool loadRecords(void);
    bool loadFiles(void);

    volatile bool m_running, m_suspended;
    QWaitCondition m_loadCond;
    QMutex m_loadMutex;

    QVariantList m_recordsList;
    QStringList m_existingsList, m_filesList;
    ViewType m_viewType;
};

#endif // UTILITY_H
