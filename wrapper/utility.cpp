#include "utility.h"
#include "child/TemplateChildWidget.h"
#include <QDebug>

Converter::Converter()
{
}

const QStringList &Converter::v2s(const QVector<QString> strVector, QStringList &strList)
{
    int size = strVector.size();
    for (int i = 0; i < size; i++)
    {
        strList << strVector.at(i);
    }
}

int Converter::num(const QStringList &strList, bool empty)
{
    int n = 0;

    if (!empty)
    {
        return strList.size();
    }

    foreach (const QString &str, strList)
    {
        if (!str.isEmpty())
        {
            n++;
        }
    }

    return n;
}

void LoaderThread::end()
{
    while (isRunning())
    {
        if (m_running)
        {
            if (!m_loadMutex.tryLock())
            {
                m_loadMutex.unlock();
                m_loadMutex.lock();
            }

            if (m_suspended)
            {
                m_suspended = false;
                m_loadCond.wakeOne();
            }

            m_running = false;
            m_loadMutex.unlock();
            wait();
        }
    }
}

void LoaderThread::run()
{
    while (m_running)
    {
        if (m_suspended)
        {
            m_loadCond.wait(&m_loadMutex);
        }

        loadRecords();
        loadFiles();
    }
}

void LoaderThread::reset()
{
    bool locked = false;

    if (isRunning())
    {
        if (!m_loadMutex.tryLock())
        {
            m_loadMutex.unlock();
            m_loadMutex.lock();
        }

        locked = true;
    }

    if (!m_suspended)
    {
        m_suspended = true;
    }
    else
    {
        m_suspended = false;
        m_loadCond.wakeOne();
    }

    if (locked)
    {
        m_loadMutex.unlock();
    }
}

bool LoaderThread::loadRecords()
{
    if (m_recordsList.isEmpty())
    {
        return false;
    }

    bool loaded = false;

    foreach (QVariant list, m_recordsList)
    {
        QVariantMap recordsMap = list.toMap();
        int index = recordsMap["index"].toInt();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "View" << m_viewType << ": go on loop the list...";

        while (!m_loadMutex.tryLock());

        //msleep(20);

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "View" << m_viewType << ": read record item.";

        //m_loadMutex.lock();

        if (m_suspended)
        {
            m_loadCond.wait(&m_loadMutex);
        }

        if (!m_running)
        {
            //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "View" << m_viewType << ": receive a exit signal!";
            m_suspended = false;    // Doesn't to wake up itself any more
            m_loadMutex.unlock();
            break;
        }

        if (ViewType_Album != m_viewType)
        {
            QString file = ViewType_Photo == m_viewType ? recordsMap["picture_file"].toString() : recordsMap["template_file"].toString();

            if (!file.isEmpty())
            {
                int usedTimes = recordsMap["used_times"].toInt();

                if (ViewType_Photo == m_viewType)
                {
                    qreal angle = recordsMap["rotation_angle"].toReal();
                    Qt::Axis axis = (Qt::Axis)recordsMap["rotation_axis"].toInt();
                    emit itemAdded(index, file, angle, axis, usedTimes);
                }
                else
                {
                    emit itemAdded(index, file, usedTimes);
                }
            }
        }
        else
        {
            QStringList photosList = recordsMap["photos_list"].toStringList();
            QString tmplFile = recordsMap["template_file"].toString();
            emit itemAdded(index, photosList, tmplFile);
        }

        loaded = true;

        m_loadMutex.unlock();
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "View" << m_viewType << ": unlock finished!";
    }

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "View" << m_viewType << ": read finished!";

    if (ViewType_Album == m_viewType)
    {
        m_running = false;
        exit();
    }
    else
    {
        m_recordsList.clear();
    }

    if (loaded)
    {
        reset();
        emit loadFinished(0);
    }

    return true;
}

bool LoaderThread::loadFiles()
{
    if (m_filesList.isEmpty())
    {
        return false;
    }

    bool loaded = false;
    int index = m_existingsList.size();

    foreach (const QString &file, m_filesList)
    {
        m_loadMutex.lock();

        if (m_suspended)
        {
            m_loadCond.wait(&m_loadMutex);
        }

        if (!m_running)
        {
            m_suspended = false;    // Doesn't to wake up itself any more
            m_loadMutex.unlock();
            break;
        }

        if (!m_existingsList.contains(file, Qt::CaseInsensitive))
        {
            m_existingsList.append(file);
            index++;

            //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "View" << m_viewType << ": load file" << file;

            if (ViewType_Photo == m_viewType)
            {
                emit itemAdded(index, file, 0, Qt::ZAxis, 0);
            }

            if (ViewType_Template == m_viewType)
            {
                emit itemAdded(index, file, 0);
            }

            loaded = true;
        }

        m_loadMutex.unlock();
    }

    m_filesList.clear();

    if (loaded)
    {
        reset();
        emit loadFinished(1);
    }

    return true;
}

void CryptThread::run()
{
    QString args = m_pkgFile + " " + m_arg;
    //qDebug() << __FILE__ << __LINE__ << m_decrypt << args;

    if (m_decrypt && !m_arg.isEmpty())
    {
        TemplateChildWidget::useZip(TemplateChildWidget::ZipUsageDecrypt,
                                    args,
                                    true);
        emit done(m_pkgFile);

        TemplateChildWidget::useZip(TemplateChildWidget::ZipUsageEncrypt,
                                    args,
                                    true);

        m_decrypt = false;
        m_arg.clear();
        emit finished();
    }
}
