#ifndef ALBUMINFOWIDGET_H
#define ALBUMINFOWIDGET_H

#include <QWidget>
#include <QVariantMap>

class AlbumManageDialog;

namespace Ui {
class AlbumInfoWidget;
}

class AlbumInfoWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit AlbumInfoWidget(bool editing, AlbumManageDialog *parent = 0);
    ~AlbumInfoWidget();

    void openWnd(const QStringList &businesses);

    void openWnd(void);

    void bindUser(const QVariantMap &user, int code);

    void bindUser(const QVariantMap &user){addUser(user);}

    void removeUser(int row);

    QString getUsersId(QString &ids);

    QString getBusinessName(void) const;

    bool isSampleAlbum(void) const;

    bool isEditing(void) const {return m_editing;}

signals:
    void accepted(bool ok,
                  const QString &business = QString(),
                  bool sample = false);

private slots:
    void on_userRadioButton_clicked(){showUsers();}

    void on_sampleRadioButton_clicked(){showUsers(false);}

    void on_okPushButton_clicked();

    void on_cancelPushButton1_clicked(){emit accepted(false);}

    void on_backPushButton_clicked(){emit accepted(false);}

    void on_cancelPushButton2_clicked();

    void on_findPushButton_clicked();

    void on_telnoLineEdit_textChanged(const QString &arg1);

    void on_renamePushButton_clicked();

    void on_setPushButton_clicked();

    void on_previewPushButton_clicked();

private:
    void showUsers(bool visiable = true);

    void addUser(const QVariantMap &user);

    Ui::AlbumInfoWidget *ui;
    AlbumManageDialog *m_container;
    QString m_name;
    bool m_editing;
};

class UserInfoWidget : public QWidget
{
    Q_OBJECT

public:
    UserInfoWidget(int index, int id, const QString &info, AlbumInfoWidget &container);

    void setIndex(int index){m_index = index;}
    int getIndex() const {return m_index;}

    int getId() const {return m_id;}

    QString getInfo(void) const {return m_info;}

private slots:
    void remove()
    {
        m_container.removeUser(m_index);
    }

private:
    AlbumInfoWidget &m_container;
    int m_index, m_id;
    QString m_info;
};

#endif // ALBUMINFOWIDGET_H
