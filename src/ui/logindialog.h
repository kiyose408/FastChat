#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include "core/IAuthService.h"
#include "core/ApiService.h"
#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_Login_btn_clicked();
    // ✅ 新增的私有槽函数
    void onLoginSuccess(const QJsonObject&, const QString&);
    void onLoginFailed(const QString&);
    void on_New_btn_clicked();

private:
    Ui::LoginDialog *ui;
    // IAuthService *m_authService = nullptr;  //如果你打算使用 MockAuthService
    ApiService* m_apiService; //存储ApiService实例
};

#endif // LOGINDIALOG_H
