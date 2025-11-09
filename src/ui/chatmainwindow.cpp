#include "chatmainwindow.h"
#include "./ui_chatmainwindow.h"
#include "core/configmanager.h"
ChatMainWindow::ChatMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatMainWindow)
{
    ui->setupUi(this);

    //恢复窗口大小
    resize(ConfigManager::instance().windowSize());
}

ChatMainWindow::~ChatMainWindow()
{
    //保存窗口大小
    ConfigManager::instance().setWindowSize(size());
    delete ui;
}
