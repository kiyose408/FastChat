#include "logindialog.h"
#include "ui_logindialog.h"
#include "RegisterDialog.h"
#include <QMessageBox>
#include <QString>
#include <utils/logger.h>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_apiService(new ApiService(this))
{
    ui->setupUi(this);

    setWindowTitle("FastChat - 登录");
    
    connect(m_apiService, &ApiService::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(m_apiService, &ApiService::loginFailed, this, &LoginDialog::onLoginFailed);
}

LoginDialog::~LoginDialog()
{
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
    
    m_apiService->login(username, password);
}

void LoginDialog::onLoginSuccess(const QJsonObject &, const QString &)
{
    accept();
}

void LoginDialog::onLoginFailed(const QString & error)
{
    QMessageBox::warning(this, "登录失败", error);
}

void LoginDialog::on_New_btn_clicked()
{
    qDebug() << "Register button clicked in LoginDialog.";
    
    RegisterDialog registerDialog(m_apiService, this);
    registerDialog.setWindowTitle("FastChat - 注册");
    registerDialog.exec();
}
