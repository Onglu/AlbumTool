#ifndef TEMPLATEPAGEWIDGET_H
#define TEMPLATEPAGEWIDGET_H

#include <QWidget>
#include <QVariant>

class QGraphicsScene;
class QGraphicsView;
class QCheckBox;
class TaskPageWidget;

namespace Ui {
class TemplatePageWidget;
}

class TemplatePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TemplatePageWidget(bool previewable, TaskPageWidget *parent = 0);
    ~TemplatePageWidget();

    QGraphicsView *getView(void) const;

    bool changeTemplate(const QVariantMap &belongings);
    const QVariantMap &getBelongings(void){return m_belongings;}

    void setTags(bool immediate, const QVariantMap &tags);
    const QVariantMap &getTags(void){return m_tags;}

    bool isImmediate(void) const;

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

    void on_searchCheckBox_clicked(bool checked);

    void on_styleLineEdit_textChanged(const QString &arg1);

    void on_resetPushButton_clicked();

private:
    void setPreviewable(bool previewable);

    void addTag(const QCheckBox *cb);

    void setTag(int type, bool checked, const QString &name = QString());

    Ui::TemplatePageWidget *ui;
    TaskPageWidget *m_container;
    const QSize m_tmplSize;
    QVariantMap m_tags, m_belongings;
};

#endif // TEMPLATEPAGEWIDGET_H
