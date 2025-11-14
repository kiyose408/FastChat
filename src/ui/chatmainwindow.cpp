#include "chatmainwindow.h"
#include "./ui_chatmainwindow.h"
#include "core/configmanager.h"
#include "core/FriendModel.h"
#include "ui/FriendDelegate.h"
#include "core/MessageModel.h"
#include "ui/MessageDelegate.h"
#include <QTimer>
#include <QVBoxLayout>
ChatMainWindow::ChatMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatMainWindow)
{
    ui->setupUi(this);

    // 初始化模型
    m_friendModel = new FriendModel(this);
    m_messageModel = new MessageModel(this);
    // 填充测试好友
    m_friendModel->addFriend({1001, "张三", true, 2});
    m_friendModel->addFriend({1002, "李四", false, 0});
    m_friendModel->addFriend({1003, "王五", true, 5});

    // 填充测试消息
    m_messageModel->addMessage({false, "你好！", "10:30"});
    m_messageModel->addMessage({true, "在呢~", "10:31"});
    m_messageModel->addMessage({false, "能加个好友吗？", "10:32"});
    m_messageModel->addMessage({true, "当然可以！", "10:33"});

    // 设置视图
    ui->friend_List_listview->setModel(m_friendModel);
    ui->friend_List_listview->setItemDelegate(new FriendDelegate(this));
    ui->chatList_listview->setModel(m_messageModel);
    ui->chatList_listview->setItemDelegate(new MessageDelegate(this));
    ui->chatList_listview->setStyleSheet("QListView { background-color: #F8F8F8; }");

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
    int id = index.data(FriendModel::IdRole).toInt();
    QString name = index.data(FriendModel::NicknameRole).toString();
    qDebug() << "Selected friend:" << id << name;

    // 示例：清空并加载新聊天
    m_messageModel->clear();
    m_messageModel->addMessage({false, QString("Hello, %1!").arg(name), "现在"});
    ui->chatList_listview->scrollToBottom();
}
