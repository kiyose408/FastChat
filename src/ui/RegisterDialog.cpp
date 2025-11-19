#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"
#include <QMessageBox>
#include <QString>
#include <utils/logger.h>
RegisterDialog::RegisterDialog(ApiService* apiService,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    ,m_apiService(apiService)
{
    qDebug() << "RegisterDialog constructor called.";
    qDebug() << "ApiService pointer passed to RegisterDialog:" << m_apiService;

    if (!m_apiService) {
        qWarning() << "Warning: ApiService pointer is null in RegisterDialog!";
        // 或者可以抛出异常或设置错误标志
        // 但这里最好在调用前就保证不为 null
    }

    ui->setupUi(this);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_Register_btn_clicked()
{
    QString newUsername = ui->userId_lineEdit->text();
    QString newEmail = ui->email_lineEdit->text();
    QString newPassword = ui->userPassword_lineEdit->text();
    QString confirmPassword = ui->confirmUserPassword_lineEdit->text();

    // 简单验证
    if (newUsername.isEmpty() || newEmail.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "错误", "所有字段都是必需的！");
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致！");
        return;
    }

    // TODO: 调用你的注册逻辑 API
    m_apiService->registerUser(newUsername,newEmail,newPassword);

// 连接注册信号
    connect(m_apiService, &ApiService::registerSuccess, this, [=](const QJsonObject &user, const QString &token){
        qDebug() << "Registration successful!";
        QMessageBox::information(this, tr("Success"), tr("Registration successful!"));
        close(); // 关闭注册窗口
});

    connect(m_apiService, &ApiService::registerFailed, this, [=](const QString &error){
    // 处理注册失败
    qDebug() << "Registration failed:" << error;
    // 例如：显示错误提示
    // 如果注册成功，接受对话框
    accept(); // 这会触发 QDialog::accepted 信号
    });
}

