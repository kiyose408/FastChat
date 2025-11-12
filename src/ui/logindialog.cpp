#include "logindialog.h"
#include "ui_logindialog.h"
#include "core/MockAuthService.h"
#include <QMessageBox>
#include <QString>
#include <utils/logger.h>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    //初始化认证服务(开发阶段用Mock)
    m_authService = new MockAuthService();
    setWindowTitle("FastChat - 登录");

    setModal(true);  //确保是模态登录
}

LoginDialog::~LoginDialog()
{
    delete m_authService;
    delete ui;
}
void LoginDialog::on_Login_btn_clicked()
{
    QString userid = ui->userId_lineEdit->text().trimmed();
    QString password = ui->userPassword_lineEdit->text().trimmed();

    Logger::info("Login attempt for user: " + userid);

    if (userid.isEmpty()) {
        Logger::warn("Login rejected: empty user ID");
        QMessageBox::warning(this, "输入错误", "请输入用户名！");
        return;
    }

    if (password.isEmpty()) {
        Logger::warn("Login rejected: empty password for user: " + userid);
        QMessageBox::warning(this, "输入错误", "请输入密码！");
        return;
    }
    if(m_authService->validateCredentials(userid,password)){
        Logger::info("User '" + userid + "' authenticated successfully.");
        accept();
    }else{
        Logger::warn("Authentication failed for user: " + userid);
        QMessageBox::warning(this,"登陆失败","用户名或密码错误！");
    }
}
