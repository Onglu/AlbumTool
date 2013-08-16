#include "TemplatePageWidget.h"
#include "ui_TemplatePageWidget.h"
#include <QDebug>
#include <QDragEnterEvent>
#include "wrapper/DraggableLabel.h"
#include "page/TaskPageWidget.h"

TemplatePageWidget::TemplatePageWidget(bool previewable, TaskPageWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplatePageWidget),
    m_container(parent),
    m_tmplSize(QSize(270, 390))
{
    Q_ASSERT(m_container);

    ui->setupUi(this);

    ui->searchCheckBox->hide();
    //ui->resetPushButton->hide();

    m_belongings["picture_file"] = "";

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

bool TemplatePageWidget::changeTemplate(const QVariantMap &belongings)
{
    bool ok = false;
    QString tmplPic = belongings["picture_file"].toString();
    QString currPic = m_belongings["picture_file"].toString();

    if (currPic != tmplPic)
    {
        ok = true;
        ui->templateLabel->clear();

        QPixmap pix(tmplPic);
        if (!pix.isNull())
        {
            ui->templateLabel->setPixmap(pix.scaled(m_tmplSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        m_belongings = belongings;
    }

    return ok;
}

QGraphicsView *TemplatePageWidget::getView() const
{
    return ui->templatesGraphicsView;
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
        QString currPic = m_belongings["picture_file"].toString();
        QVariantMap belongings = picLabel->getBelongings();
        if (changeTemplate(belongings))
        {
            event->acceptProposedAction();
            emit replaced(currPic, belongings["template_file"].toString());
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
        value = tr("版式");
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
        m_tags.insert(name, value);
    }
    else
    {
        m_tags.remove(name);
    }

    if (ui->searchCheckBox->isChecked())
    {
        on_searchPushButton_clicked();
    }
}

void TemplatePageWidget::setTag(int type, bool checked, const QString &name)
{
    if (checked && name.isEmpty())
    {
        return;
    }

    if (1 == type)
    {
        if (!checked)
        {
            ui->typeChildrenCheckBox->setChecked(checked);
            ui->typePictorialCheckBox->setChecked(checked);
            ui->typeWeddingCheckBox->setChecked(checked);
        }
        else
        {
            if (name == ui->typeChildrenCheckBox->text())
            {
                ui->typeChildrenCheckBox->setChecked(checked);
            }
            else if (name == ui->typePictorialCheckBox->text())
            {
                ui->typePictorialCheckBox->setChecked(checked);
            }
            else if (name == ui->typeWeddingCheckBox->text())
            {
                ui->typeWeddingCheckBox->setChecked(checked);
            }
        }
    }
    else if (2 == type)
    {
        if (!checked)
        {
            ui->styleFramelessCheckBox->setChecked(checked);
            ui->styleFrameCheckBox->setChecked(checked);
            ui->styleNonmaskCheckBox->setChecked(checked);
            ui->styleMaskCheckBox->setChecked(checked);
        }
        else
        {
            if (name == ui->styleFramelessCheckBox->text())
            {
                ui->styleFramelessCheckBox->setChecked(checked);
            }
            else if (name == ui->styleFrameCheckBox->text())
            {
                ui->styleFrameCheckBox->setChecked(checked);
            }
            else if (name == ui->styleNonmaskCheckBox->text())
            {
                ui->styleNonmaskCheckBox->setChecked(checked);
            }
            else if (name == ui->styleMaskCheckBox->text())
            {
                ui->styleMaskCheckBox->setChecked(checked);
            }
        }
    }
    else if (3 == type)
    {
        if (!checked)
        {
            ui->colorBlackCheckBox->setChecked(checked);
            ui->colorWhiteCheckBox->setChecked(checked);
            ui->colorGrayCheckBox->setChecked(checked);
            ui->colorCoffeeCheckBox->setChecked(checked);
            ui->colorRedCheckBox->setChecked(checked);
            ui->colorPinkCheckBox->setChecked(checked);
            ui->colorOrangeCheckBox->setChecked(checked);
            ui->colorYellowCheckBox->setChecked(checked);
            ui->colorCyanCheckBox->setChecked(checked);
            ui->colorGreenCheckBox->setChecked(checked);
            ui->colorBlueCheckBox->setChecked(checked);
            ui->colorPurpleCheckBox->setChecked(checked);
        }
        else
        {
            if (name == ui->colorBlackCheckBox->text())
            {
                ui->colorBlackCheckBox->setChecked(checked);
            }
            else if (name == ui->colorWhiteCheckBox->text())
            {
                ui->colorWhiteCheckBox->setChecked(checked);
            }
            else if (name == ui->colorGrayCheckBox->text())
            {
                ui->colorGrayCheckBox->setChecked(checked);
            }
            else if (name == ui->colorCoffeeCheckBox->text())
            {
                ui->colorCoffeeCheckBox->setChecked(checked);
            }
            if (name == ui->colorRedCheckBox->text())
            {
                ui->colorRedCheckBox->setChecked(checked);
            }
            else if (name == ui->colorPinkCheckBox->text())
            {
                ui->colorPinkCheckBox->setChecked(checked);
            }
            else if (name == ui->colorOrangeCheckBox->text())
            {
                ui->colorOrangeCheckBox->setChecked(checked);
            }
            else if (name == ui->colorYellowCheckBox->text())
            {
                ui->colorYellowCheckBox->setChecked(checked);
            }
            if (name == ui->colorCyanCheckBox->text())
            {
                ui->colorCyanCheckBox->setChecked(checked);
            }
            else if (name == ui->colorGreenCheckBox->text())
            {
                ui->colorGreenCheckBox->setChecked(checked);
            }
            else if (name == ui->colorBlueCheckBox->text())
            {
                ui->colorBlueCheckBox->setChecked(checked);
            }
            else if (name == ui->colorPurpleCheckBox->text())
            {
                ui->colorPurpleCheckBox->setChecked(checked);
            }
        }
    }
}

void TemplatePageWidget::setTags(bool immediate, const QVariantMap &tags)
{
    QVariantMap::const_iterator iter = tags.constBegin();

    m_tags = tags;

    ui->searchCheckBox->setChecked(immediate);
    if (!immediate)
    {
        ui->searchPushButton->setEnabled(true);
    }

    ui->styleLineEdit->clear();

    setTag(1, false);
    setTag(2, false);
    setTag(3, false);

    while (iter != tags.constEnd())
    {
        //qDebug() << __FILE__ << __LINE__ << iter.key() << ":" << iter.value().toString();

        if ("pagetype" == iter.key())
        {
            if (iter.value().toInt())
            {
                ui->wpCoverRadioButton->setChecked(true);
            }
            else
            {
                ui->wpPageRadioButton->setChecked(true);
            }
        }
        else
        {
            QString tag = iter.value().toString();
            if (tr("种类") == tag)
            {
                setTag(1, true, iter.key());
            }
            else if (tr("版式") == tag)
            {
                setTag(2, true, iter.key());
            }
            else if (tr("色系") == tag)
            {
                setTag(3, true, iter.key());
            }
            else if (tr("风格") == iter.key())
            {
                ui->styleLineEdit->setText(tag);
            }
        }

        ++iter;
    }


}

bool TemplatePageWidget::isImmediate(void) const
{
    return ui->searchCheckBox->isChecked();
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
    m_tags.insert("pagetype", ui->wpCoverRadioButton->isChecked() ? 1 : 0);
    //qDebug() << __FILE__ << __LINE__ << "风格 :" << style;
    m_tags.insert(tr("风格"), ui->styleLineEdit->text());
    m_container->onSearch(/*ui->searchCheckBox->isChecked(), ui->templateLabel->isVisible(), */m_tags);
}

void TemplatePageWidget::on_searchCheckBox_clicked(bool checked)
{
    ui->searchPushButton->setEnabled(!checked);
    if (checked)
    {
        on_searchPushButton_clicked();
    }
}

void TemplatePageWidget::on_resetPushButton_clicked()
{
    ui->buttonGroup->setExclusive(false);
    ui->wpCoverRadioButton->setChecked(false);
    ui->wpPageRadioButton->setChecked(false);
    ui->buttonGroup->setExclusive(true);

    ui->styleLineEdit->clear();
    ui->searchCheckBox->setChecked(false);
    ui->searchPushButton->setEnabled(true);

    setTag(1, false);
    setTag(2, false);
    setTag(3, false);

    m_tags.clear();
    m_container->onSearch();
}

void TemplatePageWidget::on_styleLineEdit_textChanged(const QString &arg1)
{
    //ui->styleLineEdit->setText(arg1);
    //qDebug() << __FILE__ << __LINE__ << "风格 :" << arg1;
}
