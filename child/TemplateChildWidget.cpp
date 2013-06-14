#include "TemplateChildWidget.h"
#include "parser/json.h"
#include "parser/FileParser.h"
#include "page/TaskPageWidget.h"
#include <QDebug>

using namespace QtJson;

QProcess TemplateChildWidget::m_tmaker;
TemplateChildWidget::ZipUsage TemplateChildWidget::m_usage = TemplateChildWidget::ZipUsageCompress;

TemplateChildWidget::TemplateChildWidget(int index,
                                         const QString &tmplFile,
                                         int usedTimes,
                                         TaskPageWidget *parent) :
    PictureChildWidget(QSize(90, 130), true, parent),
    m_tmplFile(QDir::toNativeSeparators(tmplFile))
{
    setToolTip(tr("鼠标点中以拖放"));

    setIndexLabel(index, NULL, QPoint(9, 3));

    memset(m_locations, 0, 2);

#ifndef FROM_PACKAGE
    FileParser fp(tmplFile);
    if (fp.openTemplate(m_bases, m_size, m_tags, m_layers))
    {
        m_tmplDir = m_tmplFile.left(m_tmplFile.lastIndexOf(QDir::separator()) + 1);
        m_tmplPic = QString("%1%2.psd.png").arg(m_tmplDir).arg(m_bases["name"].toString());
        m_locations[0] = (uchar)m_bases["landscapeCount"].toUInt();
        m_locations[1] = (uchar)m_bases["portraitCount"].toUInt();
    }

    setPictureLabel(m_tmplPic, QSize(72, 100), DRAGGABLE_TEMPLATE, this, QPoint(9, 21));
    setTemplate(*m_picLabel, m_tmplPic, m_tmplFile, m_bases["pagetype"].toInt(), usedTimes);

    if (usedTimes)
    {
        m_picLabel->accept(true);
    }
#else
    connect(&m_tmaker, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(&m_parser, SIGNAL(done(QString)), SLOT(ok(QString)), Qt::BlockingQueuedConnection);
    connect(&m_parser, SIGNAL(finished()), &m_parser, SLOT(quit()));

    m_parser.crypt(true, m_tmplFile, PKG_PASSWORD);
    m_parser.start();
    //qDebug() << __FILE__ << __LINE__ << &m_parser << m_parser.isRunning() << m_tmplFile;

    TaskPageWidget::showProcess(true, QRect(parent->mapToGlobal(QPoint(0, 0)), parent->size()), "正在解析相册模板...");
#endif

    m_picAttrMap.insert("template_file", m_tmplFile);
    m_picAttrMap.insert("used_times", usedTimes);

    //qDebug() << __FILE__ << __LINE__ << index << m_picLabel->getBelongings();
}

TemplateChildWidget::~TemplateChildWidget()
{
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
    m_picAttrMap.insert("used_times", belongings["used_times"].toInt());
    return PictureChildWidget::getChanges();
}

void TemplateChildWidget::ok(const QString &pkgFile)
{
    //qDebug() << __FILE__ << __LINE__ << &m_parser << m_pkgFile << pkgFile;

    if (m_tmplFile == pkgFile)
    {
        TaskPageWidget::showProcess(false);
        useZip(ZipUsageRead, m_tmplFile + " page.dat");
    }
}

void TemplateChildWidget::useZip(ZipUsage usage, const QString &arguments, bool block)
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

    m_usage = usage;

    if (!block)
    {
        m_tmaker.start(program);
        m_tmaker.waitForFinished();
    }
    else
    {
        m_tmaker.execute(program);
    }
}

void TemplateChildWidget::processFinished(int ret, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(ret);

    //qDebug() << __FILE__ << __LINE__ << &m_parser << m_parser.isFinished() << m_pkgFile << m_parser.getPkgFile();
    if (m_parser.isFinished())
    {
        return;
    }

    if (QProcess::NormalExit == exitStatus)
    {
        QByteArray out = m_tmaker.readAllStandardOutput();
        QString content(out);
        m_tmaker.close();

        if (content.startsWith("compress:"))
        {
//            moveTo(m_psdPic, m_tmpDir);
//            deleteDir(content.mid(9));
        }

        if (ZipUsageAppend == m_usage /*&& !m_tmpFile.isEmpty()*/)
        {
            //qDebug() << __FILE__ << __LINE__ << m_tmpFile;
            //access();
        }

        if (content.startsWith("data:"))
        {
            QString data = content.mid(5);
            //qDebug() << __FILE__ << __LINE__ << /*data*/ m_tmplFile;

            bool ok;
            m_data = QtJson::parse(data, ok).toMap();
            if (!ok)
            {
                return;
            }

#if 0
            QVariantMap::const_iterator iter = page.constBegin();
            while (iter != page.constEnd())
            {
                if ("name" == iter.key())
                {
                    m_currFile = m_tmplPic = QString("%1.psd.png").arg(iter.value().toString());
                    m_belongings.insert(iter.key(), m_tmplPic);
                }
                else
                {
                    if ("size" == iter.key())
                    {
                        m_size = iter.value().toMap();
                    }
                    else if ("tags" == iter.key())
                    {
                        m_tags = iter.value().toList();
                    }
                    else if ("layers" == iter.key())
                    {
                        m_layers = iter.value().toList();
                    }
                    else if ("landscapeCount" == iter.key())
                    {
                        m_locations[0] = (uchar)iter.value().toUInt();
                    }
                    else if ("portraitCount" == iter.key())
                    {
                        m_locations[1] = (uchar)iter.value().toUInt();
                    }
                    else
                    {
                        //m_bases.insert(iter.key(), iter.value());
                    }

                    m_belongings.insert(iter.key(), iter.value());
                }

                ++iter;
            }
#endif

            m_currFile = m_tmplPic = QString("%1.psd.png").arg(m_data["name"].toString());
            useZip(ZipUsageRead, m_tmplFile + " " + m_tmplPic);
            loadPictures();
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
                setPictureLabel(pix, QSize(72, 100), DRAGGABLE_TEMPLATE, this, QPoint(9, 21));
            }
            else
            {
                QFile fd(name);
                if (!pix.isNull() && fd.open(QIODevice::ReadOnly))
                {
                    m_pictures.insert(m_currFile, fd.readAll());
                }
            }

            QFile::remove(name);
        }
    }
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

void TemplateChildWidget::loadPictures()
{
    m_layers = m_data["layers"].toList();
    if (m_layers.isEmpty())
    {
        return;
    }

    foreach (const QVariant &layer, m_layers)
    {
        QVariantMap data = layer.toMap();
        m_currFile = data["id"].toString() + ".png";
        useZip(ZipUsageRead, m_tmplFile + " " + m_currFile);
        //qDebug() << __FILE__ << __LINE__ << filename;
    }

    m_belongings.insert("pictures", m_pictures);
}
