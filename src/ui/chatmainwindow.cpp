#include "chatmainwindow.h"
#include "./ui_chatmainwindow.h"
#include <QTimer>
#include <QVBoxLayout>
ChatMainWindow::ChatMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatMainWindow)
    , m_apiService(new ApiService(this))
    , m_webSocketClient(new WebSocketClient(this))
    , m_sessionManager(SessionManager::instance())
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
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
    //resize(ConfigManager::instance().windowSize());
}

void ChatMainWindow::toggleMaxmize()
{
    if (isMaximized()) {
        //resize(QSize(800,580)) ;// 还原
        showNormal();
        qDebug("showNormal");
    } else {
        showMaximized(); // 最大化

        qDebug("showMax");
    }
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

void ChatMainWindow::on_addFriend_label_clicked()
{
    FriendManagementDialog* friendManagementDialog = new FriendManagementDialog(m_apiService, this);

    // 设置窗口标志 (可选)
    friendManagementDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // 设置为模态对话框
    friendManagementDialog->setModal(true);
    // 关闭时自动删除
    friendManagementDialog->setAttribute(Qt::WA_DeleteOnClose);

    friendManagementDialog->show();

}


void ChatMainWindow::on_esc_label_clicked()
{
    ChatMainWindow::close();
}


void ChatMainWindow::on_max_label_clicked()
{
    ChatMainWindow::toggleMaxmize();
}


void ChatMainWindow::on_minus_label_clicked()
{
    ChatMainWindow::showMinimized();
}
// --- 重写的鼠标事件处理函数 ---
void ChatMainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPosition().toPoint();
        m_windowStartGeometry = geometry(); // 记录窗口开始拖动时的位置和大小
        // qDebug() << "Mouse Pressed at global pos:" << m_dragStartPosition;
    }
    QWidget::mousePressEvent(event); // 调用基类处理
}

void ChatMainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging && event->buttons() & Qt::LeftButton) {
        // 计算鼠标移动的偏移量
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPosition;
        // 更新窗口位置
        move(m_windowStartGeometry.topLeft() + delta);
        // qDebug() << "Moving window to:" << m_windowStartGeometry.topLeft() + delta;
    }
    QWidget::mouseMoveEvent(event); // 调用基类处理
}

void ChatMainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        // qDebug() << "Mouse Released";
    }
    QWidget::mouseReleaseEvent(event); // 调用基类处理
}

