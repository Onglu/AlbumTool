#ifndef TEMPLATEPAGEWIDGET_H
#define TEMPLATEPAGEWIDGET_H

#include <QWidget>

class QGraphicsScene;
class QGraphicsView;

namespace Ui {
class TemplatePageWidget;
}

class TemplatePageWidget : public QWidget
{
    Q_OBJECT   
public:
    explicit TemplatePageWidget(bool previewable, QWidget *parent = 0);
    ~TemplatePageWidget();

    void setPreview(const QString &tmplPic);

    QGraphicsView *getView(void) const;

    bool clearTmplLabel(const QString &tmplFile);

signals:
    void replaced(const QString &tmplPic, const QString &tmplFile);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void setPreviewable(bool previewable);

    Ui::TemplatePageWidget *ui;
    const QSize m_tmplSize;
    QString m_tmplPic;
};

#endif // TEMPLATEPAGEWIDGET_H
