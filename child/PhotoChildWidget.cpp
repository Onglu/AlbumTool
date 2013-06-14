#include "PhotoChildWidget.h"
#include "ui_PhotoChildWidget.h"
#include "page/TaskPageWidget.h"
#include <QGraphicsSceneMouseEvent>
#include <QDragEnterEvent>
#include <QDebug>

PhotoChildWidget::PhotoChildWidget(int index,
                                   const QString &file,
                                   qreal angle,
                                   Qt::Axis axis,
                                   int usedTimes,
                                   TaskPageWidget *parent) :
    PictureChildWidget(QSize(162, 142), true, parent),
    ui(new Ui::PhotoChildWidget)
{
    ui->setupUi(this);

    setIndexLabel(index, ui->indexLabel);

    setPictureLabel(/*file*/ QPixmap(file), QSize(141, 96), DRAGGABLE_PHOTO, ui->pixmapLabel);

    if (!m_picLabel->hasPicture())
    {
        ui->pixmapLabel->setText(tr("无法显示"));
    }
    else
    {
        rotate(angle, axis, false);
    }

    setPhoto(*m_picLabel, file, angle, axis, usedTimes);
    m_picAttrMap = m_picLabel->getBelongings();

    if (usedTimes)
    {
        m_picLabel->accept(true);
    }

    //qDebug() << __FILE__ << __LINE__ << "Photo:" << m_picLabel << usedTimes;
}

PhotoChildWidget::~PhotoChildWidget()
{
    delete ui;
}

inline void PhotoChildWidget::showButtons(bool bVisible)
{
    ui->leftRotationPushButton->setVisible(bVisible);
    ui->rightRotationButton->setVisible(bVisible);
    ui->mirroredPushButton->setVisible(bVisible);
    ui->deletePushButton->setVisible(bVisible);
}

const QVariantMap &PhotoChildWidget::getChanges()
{
    m_picAttrMap = m_picLabel->getBelongings();
    return PictureChildWidget::getChanges();
}

void PhotoChildWidget::setPhoto(DraggableLabel &label,
                                const QString &file,
                                qreal angle,
                                Qt::Axis axis,
                                int usedTimes)
{
    QVariantMap belongings;
    belongings.insert("picture_file", file);
    belongings.insert("rotation_angle", angle);
    belongings.insert("rotation_axis", axis);
    belongings.insert("used_times", usedTimes);
    label.setBelongings(belongings);
}

int PhotoChildWidget::usedTimes() const
{
    return ui->timesLabel->text().toInt();
}

void PhotoChildWidget::rotate(qreal angle, Qt::Axis axis, bool report)
{
    if (m_picLabel->hasPicture())
    {
        QPixmap pix = m_picLabel->getPicture().transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation);
        m_picLabel->setPixmap(pix);

        QVariantMap &belongings = m_picLabel->getBelongings();
        qreal current = belongings["rotation_angle"].toReal();
        belongings["rotation_angle"] = Converter::rotation(current, angle);
        belongings["rotation_axis"] = axis;

        if (report)
        {
            m_container->noticeChanged();
        }
    }
}

void PhotoChildWidget::on_leftRotationPushButton_clicked()
{
    rotate(-90.0f);
}

void PhotoChildWidget::on_rightRotationButton_clicked()
{
    rotate(90.0f);
}

void PhotoChildWidget::on_mirroredPushButton_clicked()
{
    rotate(180.0f, Qt::YAxis);
}

void PhotoChildWidget::on_deletePushButton_clicked()
{
    emit itemDetached();
}

void PhotoChildWidget::onAccept(int usedTimes)
{
    if (usedTimes)
    {
        ui->timesLabel->setNum(usedTimes);
        ui->timesLabel->setToolTip(tr("已使用次数：%1").arg(usedTimes));
    }
    else
    {
        ui->timesLabel->clear();
    }

    PictureChildWidget::onAccept(usedTimes);
}

PhotoProxyWidget::PhotoProxyWidget(PhotoChildWidget *photoWidget) :
    PictureProxyWidget(photoWidget),
    m_pChildWidget(photoWidget)
{
    Q_ASSERT(m_pChildWidget);
    m_pChildWidget->showButtons(false);
}

void PhotoProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    PictureProxyWidget::hoverEnterEvent(event);
    m_pChildWidget->showButtons(true);
}

void PhotoProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    PictureProxyWidget::hoverLeaveEvent(event);
    m_pChildWidget->showButtons(false);
}
