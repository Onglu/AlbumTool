#include "TemplatePageWidget.h"
#include "ui_TemplatePageWidget.h"
#include <QDebug>
#include <QDragEnterEvent>
#include "wrapper/DraggableLabel.h"

TemplatePageWidget::TemplatePageWidget(bool previewable, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplatePageWidget),
    m_tmplSize(QSize(270, 390))
{
    ui->setupUi(this);
    setPreviewable(previewable);
}

TemplatePageWidget::~TemplatePageWidget()
{
    delete ui;
}

inline void TemplatePageWidget::setPreviewable(bool previewable)
{
    if (!previewable)
    {
        ui->templateLabel->hide();
        ui->sideFrame->setFrameShape(QFrame::NoFrame);
        ui->sideFrame->setFixedWidth(448);
        ui->mainHorizontalLayout->setMargin(0);
        ui->innerVerticalLayout->setMargin(0);
    }
    else
    {
        ui->templateLabel->show();
        ui->sideFrame->setFrameShape(QFrame::Box);
        ui->sideFrame->setMaximumWidth(16777215);
        ui->mainHorizontalLayout->setContentsMargins(32, 0, 0, 0);
        ui->mainHorizontalLayout->setSpacing(32);
        ui->innerVerticalLayout->setMargin(9);
    }
}

void TemplatePageWidget::setPreview(const QString &tmplPic)
{
    if (m_tmplPic != tmplPic)
    {
        QPixmap pix(tmplPic);

        ui->templateLabel->clear();

        if (!pix.isNull())
        {
            ui->templateLabel->setPixmap(pix.scaled(m_tmplSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        m_tmplPic = tmplPic;
    }
}

QGraphicsView *TemplatePageWidget::getView() const
{
    return ui->templatesGraphicsView;
}

bool TemplatePageWidget::clearTmplLabel(const QString &tmplPic)
{
    if (m_tmplPic == tmplPic)
    {
        ui->templateLabel->clear();
        m_tmplPic.clear();
        return true;
    }

    return false;
}

void TemplatePageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_TEMPLATE) && !children().contains(event->source()))
    {
        event->acceptProposedAction();
    }
}

void TemplatePageWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(DRAGGABLE_TEMPLATE) && !children().contains(event->source()))
    {
        event->acceptProposedAction();
    }
}

void TemplatePageWidget::dropEvent(QDropEvent *event)
{
    DraggableLabel *picLabel = static_cast<DraggableLabel *>(event->source());
    if (picLabel->meetDragDrop(DRAGGABLE_TEMPLATE) && !children().contains(picLabel))
    {
        QVariantMap belongings = picLabel->getBelongings();
        QString tmplPic = belongings["picture_file"].toString();

        if (m_tmplPic != tmplPic)
        {
            emit replaced(m_tmplPic, belongings["template_file"].toString());
            event->acceptProposedAction();
            setPreview(tmplPic);
        }
    }
}

inline void TemplatePageWidget::addTag(const QCheckBox *cb)
{
    QString value, name = cb->objectName();

    if (name.startsWith("type"))
    {
        value = tr("种类");
    }
    else if (name.startsWith("style"))
    {
        value = tr("板式");
    }
    else if (name.startsWith("color"))
    {
        value = tr("色系");
    }
    else
    {
        return;
    }

    name = cb->text();

    if (cb->isChecked() && !value.isEmpty())
    {
        m_tagsMap.insert(name, value);
    }
    else
    {
        m_tagsMap.remove(name);
    }
}

void TemplatePageWidget::on_typeChildrenCheckBox_clicked()
{
    addTag(ui->typeChildrenCheckBox);
}

void TemplatePageWidget::on_typePictorialCheckBox_clicked()
{
    addTag(ui->typePictorialCheckBox);
}

void TemplatePageWidget::on_typeWeddingCheckBox_clicked()
{
    addTag(ui->typeWeddingCheckBox);
}

void TemplatePageWidget::on_styleFramelessCheckBox_clicked()
{
    addTag(ui->styleFramelessCheckBox);
}

void TemplatePageWidget::on_styleFrameCheckBox_clicked()
{
    addTag(ui->styleFrameCheckBox);
}

void TemplatePageWidget::on_styleNonmaskCheckBox_clicked()
{
    addTag(ui->styleNonmaskCheckBox);
}

void TemplatePageWidget::on_styleMaskCheckBox_clicked()
{
    addTag(ui->styleMaskCheckBox);
}

void TemplatePageWidget::on_colorBlackCheckBox_clicked()
{
    addTag(ui->colorBlackCheckBox);
}

void TemplatePageWidget::on_colorWhiteCheckBox_clicked()
{
    addTag(ui->colorWhiteCheckBox);
}

void TemplatePageWidget::on_colorGrayCheckBox_clicked()
{
    addTag(ui->colorGrayCheckBox);
}

void TemplatePageWidget::on_colorCoffeeCheckBox_clicked()
{
    addTag(ui->colorCoffeeCheckBox);
}

void TemplatePageWidget::on_colorRedCheckBox_clicked()
{
    addTag(ui->colorRedCheckBox);
}

void TemplatePageWidget::on_colorPinkCheckBox_clicked()
{
    addTag(ui->colorPinkCheckBox);
}

void TemplatePageWidget::on_colorOrangeCheckBox_clicked()
{
    addTag(ui->colorOrangeCheckBox);
}

void TemplatePageWidget::on_colorYellowCheckBox_clicked()
{
    addTag(ui->colorYellowCheckBox);
}

void TemplatePageWidget::on_colorCyanCheckBox_clicked()
{
    addTag(ui->colorCyanCheckBox);
}

void TemplatePageWidget::on_colorGreenCheckBox_clicked()
{
    addTag(ui->colorGreenCheckBox);
}

void TemplatePageWidget::on_colorBlueCheckBox_clicked()
{
    addTag(ui->colorBlueCheckBox);
}

void TemplatePageWidget::on_colorPurpleCheckBox_clicked()
{
    addTag(ui->colorPurpleCheckBox);
}

void TemplatePageWidget::on_searchPushButton_clicked()
{
    int cover = ui->wpCoverRadioButton->isChecked() ? 1 : 0;
    qDebug() << __FILE__ << __LINE__ << "pagetype:" << cover;

    QVariantList tagsList;
    QVariantMap::const_iterator iter = m_tagsMap.constBegin();

    while (iter != m_tagsMap.constEnd())
    {
        QVariantMap tagMap;
        tagMap.insert("name", iter.key());
        tagMap.insert("type", iter.value());
        tagsList << tagMap;
        qDebug() << __FILE__ << __LINE__ << iter.key() << ":" << iter.value().toString();
        ++iter;
    }

    QString style = ui->styleLineEdit->text();
    qDebug() << __FILE__ << __LINE__ << "风格 :" << style;
//    if (!style.isEmpty())
//    {
//        QVariantMap tagMap;
//        tagMap.insert("name", style);
//        tagMap.insert("type", tr("风格"));
//        tagsList << tagMap;
//    }
}
