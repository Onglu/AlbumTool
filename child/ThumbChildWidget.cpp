#include "ThumbChildWidget.h"
#include "AlbumChildWidget.h"
#include "PhotoChildWidget.h"
#include <QDragEnterEvent>
#include <QDebug>

QStringList ThumbChildWidget::m_photosList;

ThumbChildWidget::ThumbChildWidget(int index,
                                   const QString &mimeType,
                                   const QString &file,
                                   qreal angle,
                                   Qt::Axis axis,
                                   TaskPageWidget *parent) :
    PictureChildWidget(QSize(162, 142), true, parent)
{
    setIndexLabel(index, NULL, QPoint(11, 4));
    setPictureLabel(QPixmap(file), QSize(141, 96), mimeType, this, QPoint(11, 22));

    QPixmap pix = m_picLabel->getPicture();
    if (!pix.isNull() && (angle || Qt::ZAxis != axis))
    {
        m_picLabel->setPixmap(pix.transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation));
    }

    PhotoChildWidget::setPhoto(*m_picLabel, file, angle, axis);
    m_Belongings = m_picLabel->getBelongings();
}

void ThumbChildWidget::dropEvent(QDropEvent *event)
{
    DraggableLabel *picLabel = static_cast<DraggableLabel *>(event->source());

    if (!picLabel || m_photosList.isEmpty() || m_photosList.contains(picLabel->getPictureFile(), Qt::CaseInsensitive)
            || !meetDragDrop(event) || children().contains(picLabel))
    {
        event->ignore();
        return;
    }

    QByteArray data = event->mimeData()->data(m_picLabel->getMimeType());
    QDataStream stream(&data, QIODevice::ReadOnly);
    QPixmap pix;
    QPoint offset;

    stream >> pix >> offset;
    m_picLabel->setPixmap(pix);

    m_Belongings = picLabel->getBelongings();
    m_Belongings["used_times"] = m_Belongings["used_times"].toInt() + 1;

    QString current = m_picLabel->getPictureFile();
    m_photosList.removeOne(current);

    QString replaced = picLabel->getPictureFile();
    m_photosList.append(replaced);

    //qDebug() << __FILE__ << __LINE__ << "current:" << current << "," << ut
             //<< ", replaced:" << picLabel->getPictureFile() << "," << m_Belongings["used_times"].toInt();
    emit itemReplaced(current, replaced);

    m_picLabel->setBelongings(m_Belongings);
}
