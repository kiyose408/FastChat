#include "chatmainwindow.h"
#include "./ui_chatmainwindow.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QDateTime>
#include <QFont>
#include <QFileDialog>

ChatMainWindow::ChatMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatMainWindow)
    , m_apiService(new ApiService(this))
    , m_webSocketClient(new WebSocketClient(this))
    , m_sessionManager(SessionManager::instance())
    , m_friendRequestBadge(nullptr)
    , m_friendRequestCount(0)
    , m_currentFriendId(-1)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    
    QFont userIdFont;
    userIdFont.setBold(true);
    userIdFont.setPointSize(12);
    ui->userId_label->setFont(userIdFont);
    ui->userId_label->setText(m_sessionManager.username());
    
    m_friendModel = new FriendModel(this);
    m_messageModel = new MessageModel(this);

    ui->friend_List_listview->setModel(m_friendModel);
    ui->friend_List_listview->setItemDelegate(new FriendDelegate(this));
    ui->chatList_listview->setModel(m_messageModel);
    ui->chatList_listview->setItemDelegate(new MessageDelegate(this));
    ui->chatList_listview->setStyleSheet("QListView { background-color: #F8F8F8; }");

    connect(ui->friend_List_listview, &QListView::clicked, this, &ChatMainWindow::onFriendClicked);

    QTimer::singleShot(100, this, [this]() {
        ui->friend_List_listview->scrollToBottom();
    });
    
    m_friendRequestBadge = new QLabel(ui->addFriend_label->parentWidget());
    m_friendRequestBadge->setStyleSheet(
        "QLabel {"
        "   background-color: #FF4444;"
        "   color: white;"
        "   border-radius: 8px;"
        "   font-size: 10px;"
        "   font-weight: bold;"
        "   min-width: 16px;"
        "   max-width: 16px;"
        "   min-height: 16px;"
        "   max-height: 16px;"
        "   qproperty-alignment: AlignCenter;"
        "}"
    );
    m_friendRequestBadge->hide();
    
    connect(m_apiService, &ApiService::getFriendRequestsSuccess, this, &ChatMainWindow::onGetFriendRequestsSuccess);
    connect(m_apiService, &ApiService::getFriendRequestsFailed, this, &ChatMainWindow::onGetFriendRequestsFailed);
    connect(m_apiService, &ApiService::getFriendsSuccess, this, &ChatMainWindow::onGetFriendsSuccess);
    connect(m_apiService, &ApiService::getFriendsFailed, this, &ChatMainWindow::onGetFriendsFailed);
    connect(m_apiService, &ApiService::sendMessageSuccess, this, &ChatMainWindow::onSendMessageSuccess);
    connect(m_apiService, &ApiService::sendMessageFailed, this, &ChatMainWindow::onSendMessageFailed);
    connect(m_apiService, &ApiService::getConversationSuccess, this, &ChatMainWindow::onGetConversationSuccess);
    connect(m_apiService, &ApiService::getConversationFailed, this, &ChatMainWindow::onGetConversationFailed);
    connect(m_apiService, &ApiService::deleteFriendSuccess, this, &ChatMainWindow::onDeleteFriendSuccess);
    connect(m_apiService, &ApiService::uploadImageSuccess, this, &ChatMainWindow::onUploadImageSuccess);
    connect(m_apiService, &ApiService::uploadImageFailed, this, &ChatMainWindow::onUploadImageFailed);
    connect(m_apiService, &ApiService::uploadFileSuccess, this, &ChatMainWindow::onUploadFileSuccess);
    connect(m_apiService, &ApiService::uploadFileFailed, this, &ChatMainWindow::onUploadFileFailed);
    
    connect(m_webSocketClient, &WebSocketClient::connected, this, &ChatMainWindow::onWebSocketConnected);
    connect(m_webSocketClient, &WebSocketClient::disconnected, this, &ChatMainWindow::onWebSocketDisconnected);
    connect(m_webSocketClient, &WebSocketClient::errorOccurred, this, &ChatMainWindow::onWebSocketError);
    connect(m_webSocketClient, &WebSocketClient::messageReceived, this, &ChatMainWindow::onWebSocketMessageReceived);
    connect(m_webSocketClient, &WebSocketClient::messageSent, this, &ChatMainWindow::onWebSocketMessageSent);
    connect(m_webSocketClient, &WebSocketClient::friendRequestReceived, this, &ChatMainWindow::onWebSocketFriendRequestReceived);
    connect(m_webSocketClient, &WebSocketClient::friendRequestAccepted, this, &ChatMainWindow::onWebSocketFriendRequestAccepted);
    connect(m_webSocketClient, &WebSocketClient::friendDeleted, this, &ChatMainWindow::onWebSocketFriendDeleted);
    
    QTimer::singleShot(500, this, [this]() {
        m_apiService->getFriendRequests();
        m_apiService->getFriends();
        
        QString token = m_sessionManager.token();
        if (!token.isEmpty()) {
            m_webSocketClient->connectToServer(token);
        }
    });
}

void ChatMainWindow::toggleMaxmize()
{
    if (isMaximized()) {
        showNormal();
        qDebug("showNormal");
    } else {
        showMaximized();
        qDebug("showMax");
    }
}

ChatMainWindow::~ChatMainWindow()
{
    ConfigManager::instance().setWindowSize(size());
    delete ui;
}

void ChatMainWindow::updateFriendRequestBadge(int count)
{
    m_friendRequestCount = count;
    
    if (count > 0) {
        QString displayText = count > 99 ? "99+" : QString::number(count);
        m_friendRequestBadge->setText(displayText);
        m_friendRequestBadge->show();
        
        QWidget* parent = ui->addFriend_label->parentWidget();
        QRect labelRect = ui->addFriend_label->geometry();
        int badgeX = labelRect.right() - 8;
        int badgeY = labelRect.top() - 2;
        m_friendRequestBadge->move(badgeX, badgeY);
        m_friendRequestBadge->raise();
    } else {
        m_friendRequestBadge->hide();
    }
}

void ChatMainWindow::refreshFriendList()
{
    m_apiService->getFriends();
}

void ChatMainWindow::onFriendClicked(const QModelIndex &index)
{
    m_currentFriendId = index.data(FriendModel::IdRole).toInt();
    m_currentFriendName = index.data(FriendModel::NicknameRole).toString();
    qDebug() << "Selected friend:" << m_currentFriendId << m_currentFriendName;

    ui->sessionUserId_label->setText(m_currentFriendName);
    
    m_apiService->getConversation(m_currentFriendId);
}

void ChatMainWindow::on_addFriend_label_clicked()
{
    FriendManagementDialog* friendManagementDialog = new FriendManagementDialog(m_apiService, this);

    friendManagementDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    friendManagementDialog->setModal(true);
    friendManagementDialog->setAttribute(Qt::WA_DeleteOnClose);

    friendManagementDialog->show();
    
    connect(friendManagementDialog, &QDialog::destroyed, this, [this]() {
        m_apiService->getFriendRequests();
        m_apiService->getFriends();
    });

}

void ChatMainWindow::on_more_label_clicked()
{
    if (m_currentFriendId <= 0) {
        qDebug() << "请先选择一个好友";
        return;
    }
    
    FriendInfoDialog* friendInfoDialog = new FriendInfoDialog(m_apiService, m_currentFriendId, m_currentFriendName, this);
    friendInfoDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    friendInfoDialog->setModal(true);
    friendInfoDialog->setAttribute(Qt::WA_DeleteOnClose);
    friendInfoDialog->show();
}

void ChatMainWindow::on_pic_label_clicked()
{
    if (m_currentFriendId <= 0) {
        qDebug() << "请先选择一个好友";
        return;
    }
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp *.webp)");
    if (!filePath.isEmpty()) {
        qDebug() << "选择的图片:" << filePath;
        m_apiService->uploadImage(filePath);
    }
}

void ChatMainWindow::on_file_label_clicked()
{
    if (m_currentFriendId <= 0) {
        qDebug() << "请先选择一个好友";
        return;
    }
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*.*)");
    if (!filePath.isEmpty()) {
        qDebug() << "选择的文件:" << filePath;
        m_apiService->uploadFile(filePath);
    }
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

void ChatMainWindow::onGetFriendRequestsSuccess(const QJsonArray& requests)
{
    qDebug() << "主界面获取好友请求成功，共" << requests.size() << "个请求";
    updateFriendRequestBadge(requests.size());
}

void ChatMainWindow::onGetFriendRequestsFailed(const QString& error)
{
    qDebug() << "主界面获取好友请求失败:" << error;
}

void ChatMainWindow::onGetFriendsSuccess(const QJsonArray& friends)
{
    qDebug() << "获取好友列表成功，共" << friends.size() << "个好友";
    
    m_friendModel->clear();
    
    for (const QJsonValue& friendValue : friends) {
        QJsonObject friendObj = friendValue.toObject();
        int friendId = friendObj["friend_id"].toInt();
        QString username = friendObj["username"].toString();
        
        FriendData friendData;
        friendData.id = friendId;
        friendData.nickname = username;
        friendData.isOnline = false;
        friendData.unreadCount = 0;
        
        m_friendModel->addFriend(friendData);
        qDebug() << "添加好友:" << friendId << username;
    }
}

void ChatMainWindow::onGetFriendsFailed(const QString& error)
{
    qDebug() << "获取好友列表失败:" << error;
}

void ChatMainWindow::on_send_btn_clicked()
{
    if (m_currentFriendId <= 0) {
        qDebug() << "请先选择一个好友";
        return;
    }
    
    QString content = ui->text_textEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        qDebug() << "消息内容不能为空";
        return;
    }
    
    if (m_webSocketClient->isConnected()) {
        m_webSocketClient->sendMessage(m_currentFriendId, content);
    } else {
        qDebug() << "WebSocket未连接，使用HTTP发送";
        m_apiService->sendMessage(m_currentFriendId, content);
    }
}

void ChatMainWindow::onSendMessageSuccess(const QJsonObject& message)
{
    qDebug() << "消息发送成功:" << message;
    
    ui->text_textEdit->clear();
    
    QJsonObject data = message["data"].toObject();
    QString content = data["content"].toString();
    QString time = QDateTime::fromString(data["created_at"].toString(), Qt::ISODate).toString("hh:mm");
    
    m_messageModel->addMessage({true, content, time});
    ui->chatList_listview->scrollToBottom();
}

void ChatMainWindow::onSendMessageFailed(const QString& error)
{
    qDebug() << "消息发送失败:" << error;
}

void ChatMainWindow::onGetConversationSuccess(const QJsonArray& messages)
{
    qDebug() << "获取对话记录成功，共" << messages.size() << "条消息";
    
    m_messageModel->clear();
    
    int currentUserId = m_sessionManager.userId();
    
    for (const QJsonValue& msgValue : messages) {
        QJsonObject msgObj = msgValue.toObject();
        int senderId = msgObj["sender_id"].toInt();
        QString content = msgObj["content"].toString();
        QString time = QDateTime::fromString(msgObj["created_at"].toString(), Qt::ISODate).toString("hh:mm");
        QString messageType = msgObj["message_type"].toString("text");
        QString fileUrl = msgObj["file_url"].toString();
        QString fileName = msgObj["file_name"].toString();
        
        bool isMine = (senderId == currentUserId);
        
        MessageData msgData;
        msgData.isSelf = isMine;
        msgData.text = content;
        msgData.time = time;
        msgData.messageType = messageType;
        msgData.fileUrl = fileUrl;
        msgData.fileName = fileName;
        
        m_messageModel->addMessage(msgData);
    }
    
    ui->chatList_listview->scrollToBottom();
}

void ChatMainWindow::onGetConversationFailed(const QString& error)
{
    qDebug() << "获取对话记录失败:" << error;
}

void ChatMainWindow::onDeleteFriendSuccess()
{
    qDebug() << "好友删除成功，刷新好友列表";
    m_currentFriendId = -1;
    m_currentFriendName.clear();
    ui->sessionUserId_label->clear();
    m_messageModel->clear();
    m_apiService->getFriends();
}

void ChatMainWindow::onWebSocketConnected()
{
    qDebug() << "WebSocket已连接";
}

void ChatMainWindow::onWebSocketDisconnected()
{
    qDebug() << "WebSocket已断开";
}

void ChatMainWindow::onWebSocketError(const QString& error)
{
    qWarning() << "WebSocket错误:" << error;
}

void ChatMainWindow::onWebSocketMessageReceived(const QJsonObject& message)
{
    qDebug() << "收到WebSocket消息:" << message;
    
    int senderId = message["sender_id"].toInt();
    QString content = message["content"].toString();
    QString time = QDateTime::fromString(message["created_at"].toString(), Qt::ISODate).toString("hh:mm");
    QString messageType = message["message_type"].toString("text");
    QString fileUrl = message["file_url"].toString();
    QString fileName = message["file_name"].toString();
    
    if (senderId == m_currentFriendId) {
        MessageData msgData;
        msgData.isSelf = false;
        msgData.text = content;
        msgData.time = time;
        msgData.messageType = messageType;
        msgData.fileUrl = fileUrl;
        msgData.fileName = fileName;
        
        m_messageModel->addMessage(msgData);
        ui->chatList_listview->scrollToBottom();
    } else {
        qDebug() << "收到其他用户消息，发送者ID:" << senderId;
    }
}

void ChatMainWindow::onWebSocketMessageSent(const QJsonObject& message)
{
    qDebug() << "WebSocket消息发送成功:" << message;
    
    ui->text_textEdit->clear();
    
    QString content = message["content"].toString();
    QString time = QDateTime::fromString(message["created_at"].toString(), Qt::ISODate).toString("hh:mm");
    QString messageType = message["message_type"].toString("text");
    QString fileUrl = message["file_url"].toString();
    QString fileName = message["file_name"].toString();
    
    MessageData msgData;
    msgData.isSelf = true;
    msgData.text = content;
    msgData.time = time;
    msgData.messageType = messageType;
    msgData.fileUrl = fileUrl;
    msgData.fileName = fileName;
    
    m_messageModel->addMessage(msgData);
    ui->chatList_listview->scrollToBottom();
}

void ChatMainWindow::onWebSocketFriendRequestReceived(int userId, const QString& username, const QString& note)
{
    qDebug() << "收到好友请求，用户:" << username << "备注:" << note;
    m_friendRequestCount++;
    updateFriendRequestBadge(m_friendRequestCount);
}

void ChatMainWindow::onWebSocketFriendRequestAccepted(int friendId, const QString& username)
{
    qDebug() << "好友请求被接受，用户:" << username;
    m_apiService->getFriends();
}

void ChatMainWindow::onWebSocketFriendDeleted(int friendId)
{
    qDebug() << "被好友删除，好友ID:" << friendId;
    
    if (friendId == m_currentFriendId) {
        m_currentFriendId = -1;
        m_currentFriendName.clear();
        ui->sessionUserId_label->clear();
        m_messageModel->clear();
    }
    
    m_apiService->getFriends();
}

void ChatMainWindow::onUploadImageSuccess(const QJsonObject& result)
{
    qDebug() << "图片上传成功:" << result;
    
    QJsonObject fileObj = result["file"].toObject();
    QString fileUrl = fileObj["url"].toString();
    QString fileName = fileObj["name"].toString();
    
    if (m_webSocketClient->isConnected()) {
        m_webSocketClient->sendFileMessage(m_currentFriendId, fileUrl, fileName, "image");
    }
}

void ChatMainWindow::onUploadImageFailed(const QString& error)
{
    qDebug() << "图片上传失败:" << error;
}

void ChatMainWindow::onUploadFileSuccess(const QJsonObject& result)
{
    qDebug() << "文件上传成功:" << result;
    
    QJsonObject fileObj = result["file"].toObject();
    QString fileUrl = fileObj["url"].toString();
    QString fileName = fileObj["name"].toString();
    
    if (m_webSocketClient->isConnected()) {
        m_webSocketClient->sendFileMessage(m_currentFriendId, fileUrl, fileName, "file");
    }
}

void ChatMainWindow::onUploadFileFailed(const QString& error)
{
    qDebug() << "文件上传失败:" << error;
}

void ChatMainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPosition().toPoint();
        m_windowStartGeometry = geometry();
    }
    QWidget::mousePressEvent(event);
}

void ChatMainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging && event->buttons() & Qt::LeftButton) {
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPosition;
        move(m_windowStartGeometry.topLeft() + delta);
    }
    QWidget::mouseMoveEvent(event);
}

void ChatMainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}
