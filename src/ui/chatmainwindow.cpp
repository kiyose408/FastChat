#include "chatmainwindow.h"
#include "./ui_chatmainwindow.h"
#include "core/configmanager.h"
#include "core/FriendModel.h"
#include "ui/FriendDelegate.h"
#include <QTimer>
#include <QVBoxLayout>
ChatMainWindow::ChatMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatMainWindow)
{
    ui->setupUi(this);

    // 初始化模型
    m_friendModel = new FriendModel(this);
    // 填充测试好友
    m_friendModel->addFriend({1001, "张三", true, 2});
    m_friendModel->addFriend({1002, "李四", false, 0});
    m_friendModel->addFriend({1003, "王五", true, 5});

    // 设置视图
    ui->friend_List_listview->setModel(m_friendModel);
    ui->friend_List_listview->setItemDelegate(new FriendDelegate(this));

    //链接点击事件
    connect(ui->friend_List_listview, &QListView::clicked, this, &ChatMainWindow::onFriendClicked);

    //自动滚动到底部
    QTimer::singleShot(100, this, [this]() {
        ui->friend_List_listview->scrollToBottom();
    });
    //恢复窗口大小
    resize(ConfigManager::instance().windowSize());
}

ChatMainWindow::~ChatMainWindow()
{
    //保存窗口大小
    ConfigManager::instance().setWindowSize(size());
    delete ui;
}

void ChatMainWindow::onFriendClicked(const QModelIndex &index)
{

}
