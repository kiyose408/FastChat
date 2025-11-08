#include "chatmainwindow.h"
#include "./ui_chatmainwindow.h"

ChatMainWindow::ChatMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatMainWindow)
{
    ui->setupUi(this);
}

ChatMainWindow::~ChatMainWindow()
{
    delete ui;
}
