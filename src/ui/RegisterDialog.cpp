#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"
#include <QMessageBox>
#include <QString>
#include <utils/logger.h>

RegisterDialog::RegisterDialog(ApiService* apiService, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , m_apiService(apiService)
{
    qDebug() << "RegisterDialog constructor called.";
    qDebug() << "ApiService pointer passed to RegisterDialog:" << m_apiService;

    ui->setupUi(this);
    
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowTitle("FastChat - 注册");
    setFixedSize(size());
    
    connect(ui->closeBtn, &QPushButton::clicked, this, &RegisterDialog::reject);
    
    connect(m_apiService, &ApiService::registerSuccess, this, [this](const QJsonObject &user, const QString &token){
        qDebug() << "Registration successful!";
        QMessageBox::information(this, "成功", "注册成功！请登录。");
        accept();
    });

    connect(m_apiService, &ApiService::registerFailed, this, [this](const QString &error){
        qDebug() << "Registration failed:" << error;
        QMessageBox::warning(this, "注册失败", error);
    });
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_Register_btn_clicked()
{
    QString newUsername = ui->userId_lineEdit->text().trimmed();
    QString newEmail = ui->email_lineEdit->text().trimmed();
    QString newPassword = ui->userPassword_lineEdit->text();
    QString confirmPassword = ui->confirmUserPassword_lineEdit->text();

    if (newUsername.isEmpty() || newEmail.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "错误", "所有字段都是必需的！");
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致！");
        return;
    }

    if (!newEmail.contains('@') || !newEmail.contains('.')) {
        QMessageBox::warning(this, "错误", "请输入有效的邮箱地址！");
        return;
    }

    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "错误", "密码长度至少为6位！");
        return;
    }

    if (!m_apiService) {
        QMessageBox::critical(this, "错误", "内部错误：无法连接到服务器！");
        return;
    }

    m_apiService->registerUser(newUsername, newEmail, newPassword);
}
