#include "logindialog.h"
#include "ui_logindialog.h"
#include "RegisterDialog.h"
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


void LoginDialog::on_New_btn_clicked()
{
    qDebug() << "Register button clicked in LoginDialog.";
    qDebug() << "m_apiService pointer in LoginDialog:" << m_apiService;

    // 检查 m_apiService 是否为空
    if (!m_apiService) {
        qCritical() << "ERROR: m_apiService is null in LoginDialog!";
        QMessageBox::critical(this, tr("Error"), tr("Internal error: Unable to open registration dialog."));
        return;
    }

    // 创建注册对话框
    RegisterDialog* registerDialog = new RegisterDialog(m_apiService, this);// 设置父窗口
    // 可选：设置为模态
    registerDialog->setModal(true);

    // 连接注册完成信号（如果需要）
    QObject::connect(registerDialog, &QDialog::accepted, this, [this]() {
        // 用户点击了“注册”按钮（或按下回车等）
        // 可以在这里做一些事情，比如刷新登录界面数据
        qDebug() << "Registration completed, back to login.";
        // 你可以在这里重新显示登录窗口，或者跳转到主界面
        // 例如：this->show(); // 如果你之前隐藏了它
    });

    // 显示对话框
    registerDialog->exec(); // exec() 是模态显示，阻塞直到关闭
    // 或者使用 show() 如果你希望非模态
    // registerDialog->show();
}

