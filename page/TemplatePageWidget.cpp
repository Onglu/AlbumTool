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
