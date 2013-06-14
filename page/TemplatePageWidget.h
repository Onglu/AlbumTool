#ifndef TEMPLATEPAGEWIDGET_H
#define TEMPLATEPAGEWIDGET_H

#include <QWidget>
#include <QVariant>

class QGraphicsScene;
class QGraphicsView;
class QCheckBox;

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

private slots:
    void on_typeChildrenCheckBox_clicked();

    void on_typePictorialCheckBox_clicked();

    void on_typeWeddingCheckBox_clicked();

    void on_styleFramelessCheckBox_clicked();

    void on_styleFrameCheckBox_clicked();

    void on_styleNonmaskCheckBox_clicked();

    void on_styleMaskCheckBox_clicked();

    void on_colorBlackCheckBox_clicked();

    void on_colorWhiteCheckBox_clicked();

    void on_colorGrayCheckBox_clicked();

    void on_colorCoffeeCheckBox_clicked();

    void on_colorRedCheckBox_clicked();

    void on_colorPinkCheckBox_clicked();

    void on_colorOrangeCheckBox_clicked();

    void on_colorYellowCheckBox_clicked();

    void on_colorCyanCheckBox_clicked();

    void on_colorGreenCheckBox_clicked();

    void on_colorBlueCheckBox_clicked();

    void on_colorPurpleCheckBox_clicked();

    void on_searchPushButton_clicked();

private:
    void setPreviewable(bool previewable);

    void addTag(const QCheckBox *cb);

    Ui::TemplatePageWidget *ui;
    const QSize m_tmplSize;
    QString m_tmplPic;
    QVariantMap m_tagsMap;
};

#endif // TEMPLATEPAGEWIDGET_H
