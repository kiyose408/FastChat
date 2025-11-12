#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include "core/IAuthService.h"
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

private:
    Ui::LoginDialog *ui;
    IAuthService *m_authService = nullptr;
};

#endif // LOGINDIALOG_H
