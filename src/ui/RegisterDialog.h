#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "core/ApiService.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(ApiService* apiService,QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_Register_btn_clicked();

private:
    Ui::RegisterDialog *ui;
    ApiService* m_apiService; //存储ApiService实例
};

#endif // REGISTERDIALOG_H
