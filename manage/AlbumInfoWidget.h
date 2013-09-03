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

    // 打开用户信息设置窗口
    void openWnd(const QStringList &businesses);

    // 打开用户信息设置窗口
    void openWnd(void);

    // 绑定指定用户的错误码
    void bindUser(const QVariantMap &user, int code);

    // 绑定指定用户的基本信息
    void bindUser(const QVariantMap &user){addUser(user);}

    // 移除用户
    void removeUser(int row);

    // 获取用户列表ID
    QString getUsersId(QString &ids);

    // 获取影楼商家列表
    QString getBusinessName(void) const;

    // 检查当前相册是否为样册
    bool isSampleAlbum(void) const;

    // 检查当前相册是否可编辑
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

    void on_nameLineEdit_returnPressed();

    void on_renamePushButton_clicked();

    void on_setPushButton_clicked();

    void on_previewPushButton_clicked();

private:
    // 显示用户信息列表
    void showUsers(bool visiable = true);

    // 添加用户信息
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

    // 设置/获取用户索引
    void setIndex(int index){m_index = index;}
    int getIndex() const {return m_index;}

    // 获取用户ID
    int getId() const {return m_id;}

    // 获取用户信息
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
