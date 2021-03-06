#ifndef UTILITY_H
#define UTILITY_H

#include <QVector>
#include <QStringList>
#include <QVariant>
#include <QProcess>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "defines.h"

class Converter
{
public:
    Converter(){}

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

    static const QString &getFileName(QString fullPath,
                                      QString &fileName,
                                      bool suffix);
};

class SqlHelper
{
public:
    SqlHelper(){}

    bool connectDb(void);

    const QString &getError(void) const {return m_error;}

    static int getId(const QString &sql);

    static int getLastRowId(const QString &table);

private:
    QString m_error;
};

namespace TaskLoader
{
    class LoaderThread : public QThread
    {
        Q_OBJECT

    public:
        LoaderThread(ViewType view) : QThread(), m_running(false), m_suspended(false), m_viewType(view){}
        LoaderThread(const LoaderThread &loader) : QThread(), m_running(false), m_suspended(false)
        {
            m_viewType = loader.m_viewType;
        }
        ~LoaderThread(void){end();}

        void loadList(const QVariantList &recordsList){m_recordsList = recordsList;}
        void loadList(const QStringList &existingsList, const QStringList &filesList)
        {
            m_existingsList = existingsList;
            m_filesList = filesList;
        }

        void begin(void)
        {
            if (!m_recordsList.isEmpty() || !m_filesList.isEmpty())
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

        void itemAdded(int index,
                       //const QStringList &filesList,
                       const QVariantList &filesList,
                       const QString &file/* Template data file */,
                       const QVariantList &changes);

        void done(uchar state);

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
}

#endif // UTILITY_H
