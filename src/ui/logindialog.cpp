#include "logindialog.h"
#include "ui_logindialog.h"
// #include "core/MockAuthService.h"
#include <QMessageBox>
#include <QString>
#include <utils/logger.h>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
//初始化ApiService作为本对象的子对象
    ,m_apiService(new ApiService(this))
{
    ui->setupUi(this);

    setWindowTitle("FastChat - 登录");
    // ✅ 在构造函数中连接信号，只连接一次
    connect(m_apiService, &ApiService::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(m_apiService, &ApiService::loginFailed, this, &LoginDialog::onLoginFailed);
}

LoginDialog::~LoginDialog()
{
    //delete m_authService;
    delete ui;
}
void LoginDialog::on_Login_btn_clicked()
{
    QString username = ui->userId_lineEdit->text().trimmed();
    QString password = ui->userPassword_lineEdit->text().trimmed();

    Logger::info("Login attempt for user: " + username);

    if (username.isEmpty()) {
        Logger::warn("Login rejected: empty user ID");
        QMessageBox::warning(this, "输入错误", "请输入用户名！");
        return;
    }

    if (password.isEmpty()) {
        Logger::warn("Login rejected: empty password for user: " + username);
        QMessageBox::warning(this, "输入错误", "请输入密码！");
        return;
    }
    // ✅ 直接调用 login 即可，无需重新连接信号
    m_apiService->login(username, password); // 调用登录方法

    // // 这里先保留简单方式
    // connect(m_apiService, &ApiService::loginSuccess, this, [this](const QJsonObject&, const QString&) {
    //     accept(); // 登录成功，关闭对话框
    // });
    // connect(m_apiService, &ApiService::loginFailed, this, [this](const QString& error) {
    //     QMessageBox::warning(this, "登录失败", error);
    // });
}

void LoginDialog::onLoginSuccess(const QJsonObject &, const QString &)
{
    // 登录成功，关闭对话框
    accept();
    // 注意：不要在这里 deleteLater() m_apiService，因为它是 this 的子对象
    // Qt 会在 this 销毁时自动清理子对象
}

void LoginDialog::onLoginFailed(const QString & error)
{
    QMessageBox::warning(this, "登录失败", error);
}

