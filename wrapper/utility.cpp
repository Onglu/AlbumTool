#include "utility.h"
#include <QDebug>
#include <QtSql>

using namespace TaskLoader;

const QStringList &Converter::v2l(const QVector<QString> strVector, QStringList &strList)
{
    int size = strVector.size();
    for (int i = 0; i < size; i++)
    {
        strList << strVector.at(i);
    }
}

const QString &Converter::getFileName(QString fullPath, QString &fileName, bool suffix)
{
    if (!fullPath.isEmpty())
    {
        fullPath = QDir::toNativeSeparators(fullPath);
        int pos = -1, start = fullPath.lastIndexOf(QDir::separator()) + 1;

        if (!suffix && -1 != (pos = fullPath.lastIndexOf('.')))
        {
            pos = pos - start;
        }

        fileName = fullPath.mid(start, pos);
    }

    return fileName;
}

bool SqlHelper::connectDb()
{
    static bool connected = false;

    if (!connected)
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(BACKEND_DB);
        if (!(connected = db.open()))
        {
            m_error = db.lastError().databaseText();
            return false;
        }
    }

    return connected;
}

int SqlHelper::getId(const QString &sql)
{
    SqlHelper sh;

    if (sh.connectDb())
    {
        QSqlQuery query;

        query.exec(sql);
        while (query.next())
        {
            return query.value(0).toInt();
        }
    }

    return 0;
}

int SqlHelper::getLastRowId(const QString &table)
{
    QSqlQuery query;
    QString sql = QString("select id from %1 where id=(select max(id) from %2)").arg(table).arg(table);

    query.exec(sql);
    while (query.next())
    {
        return query.value(0).toInt();
    }

    return 0;
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

        QVariantMap records = list.toMap();
        int index = records["index"].toInt();

        if (ViewType_Album != m_viewType)
        {
            QString file = ViewType_Photo == m_viewType ? records["picture_file"].toString() : records["template_file"].toString();

            if (!file.isEmpty())
            {
                int usedTimes = records["used_times"].toInt();

                if (ViewType_Photo == m_viewType)
                {
                    qreal angle = records["rotation_angle"].toReal();
                    Qt::Axis axis = (Qt::Axis)records["rotation_axis"].toInt();
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
            emit itemAdded(index, records["photos_info"].toList(), records["template_file"].toString(), records["photo_layers"].toList());
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
        emit done(LOAD_RECORDS);
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

            if (!loaded)
            {
                loaded = true;
                emit done(LOAD_NEW);
            }

            if (ViewType_Photo == m_viewType)
            {
                emit itemAdded(index, file, 0, Qt::ZAxis, 0);
            }

            if (ViewType_Template == m_viewType)
            {
                emit itemAdded(index, file, 0);
            }
        }

        m_loadMutex.unlock();
    }

    m_filesList.clear();

    if (loaded)
    {
        reset();
        emit done(LOAD_FILES);
    }

    return true;
}

void CryptThread::run()
{
    QString args = m_pkgFile + " " + m_arg;
    //qDebug() << __FILE__ << __LINE__ << m_decrypt << args;

    if (m_decrypt && !m_arg.isEmpty())
    {
//        TemplateChildWidget::useZip(TemplateChildWidget::ZipUsageDecrypt,
//                                    args,
//                                    true);
//        emit done(m_pkgFile);

//        TemplateChildWidget::useZip(TemplateChildWidget::ZipUsageEncrypt,
//                                    args,
//                                    true);

//        m_decrypt = false;
//        m_arg.clear();
//        emit finished();
    }
}
