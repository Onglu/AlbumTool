#include "TemplateChildWidget.h"
#include "parser/json.h"
#include "parser/FileParser.h"
#include <QDebug>

using namespace QtJson;

TemplateChildWidget::TemplateChildWidget(int index,
                                         const QString &tmplFile,
                                         int usedTimes,
                                         TaskPageWidget *parent) :
    PictureChildWidget(QSize(90, 130), true, parent)
{
    setToolTip(tr("鼠标点中以拖放"));

    setIndexLabel(index, NULL, QPoint(9, 3));

    memset(m_locations, 0, 2);

    FileParser fp(tmplFile);
    if (fp.openTemplate(m_bases, m_size, m_tags, m_layers))
    {
        m_tmplFile = QDir::toNativeSeparators(tmplFile);
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

    m_picAttrMap.insert("template_file", tmplFile);
    m_picAttrMap.insert("used_times", usedTimes);

    //qDebug() << __FILE__ << __LINE__ << index << m_picLabel->getBelongings();
}

void TemplateChildWidget::setTemplate(DraggableLabel &label,
                                      const QString &tmplPic,
                                      const QString &tmplFile,
                                      int cover,
                                      int used_times)
{
    QVariantMap belongings;
    belongings["picture_file"] = tmplPic;
    belongings["template_file"] = tmplFile;
    belongings["page_type"] = cover;
    belongings["used_times"] = used_times;
    label.setBelongings(belongings);
}

const QVariantMap &TemplateChildWidget::getChanges(void)
{
    QVariantMap belongings = m_picLabel->getBelongings();
    m_picAttrMap.insert("used_times", belongings["used_times"].toInt());
    return PictureChildWidget::getChanges();
}
