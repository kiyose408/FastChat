#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("FastChat - 登录");
    setModal(true);  //确保是模态登录
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
void LoginDialog::on_Login_btn_clicked()
{
    QString userId = ui->userId_lineEdit->text().trimmed();
    QString password = ui->userPassword_lineEdit->text().trimmed();

    if (userId.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名！");
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码！");
        return;
    }
    accept();
}
