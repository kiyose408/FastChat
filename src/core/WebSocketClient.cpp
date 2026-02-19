#include "WebSocketClient.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QtMath>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent)
    , m_messageQueue(100)
{
    connect(&m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
}

void WebSocketClient::connectToServer(const QString& token)
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        qDebug() << "WebSocket already connected";
        return;
    }
    
    m_token = token;
    m_state = ConnectionState::Connecting;
    
    QUrl url(AppConfig::wsBaseUrl() + "/websocket?token=" + token);
    qDebug() << "Connecting to WebSocket:" << url.toString();
    m_socket.open(url);
}

void WebSocketClient::disconnectFromServer()
{
    m_reconnectConfig.autoReconnect = false;
    stopHeartbeat();
    
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        m_socket.close();
    }
    
    m_state = ConnectionState::Disconnected;
}

bool WebSocketClient::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

ConnectionState WebSocketClient::connectionState() const
{
    return m_state;
}

void WebSocketClient::setHeartbeatConfig(const HeartbeatConfig& config)
{
    m_heartbeatConfig = config;
}

void WebSocketClient::setReconnectConfig(const ReconnectConfig& config)
{
    m_reconnectConfig = config;
}

void WebSocketClient::sendMessage(int recipientId, const QString& text)
{
    QJsonObject json;
    json["type"] = "send_message";
    json["recipientId"] = recipientId;
    json["content"] = text;

    if (!isConnected()) {
        qDebug() << "WebSocket not connected, queuing message";
        m_messageQueue.enqueue(json);
        emit messageError("Message queued, will send when connected.");
        return;
    }

    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "Message sent via WebSocket:" << json;
}

void WebSocketClient::sendFileMessage(int recipientId, const QString& fileUrl, const QString& fileName, const QString& messageType)
{
    QJsonObject json;
    json["type"] = "send_file";
    json["recipientId"] = recipientId;
    json["fileUrl"] = fileUrl;
    json["fileName"] = fileName;
    json["messageType"] = messageType;

    if (!isConnected()) {
        qDebug() << "WebSocket not connected, queuing file message";
        m_messageQueue.enqueue(json);
        emit messageError("File message queued, will send when connected.");
        return;
    }

    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "File message sent via WebSocket:" << json;
}

void WebSocketClient::markMessagesAsRead(int senderId)
{
    if (!isConnected()) {
        qWarning() << "WebSocket not connected!";
        return;
    }

    QJsonObject json;
    json["type"] = "mark_read";
    json["senderId"] = senderId;

    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "Mark read sent via WebSocket:" << json;
}

void WebSocketClient::recallMessage(int messageId, int recipientId)
{
    if (!isConnected()) {
        qWarning() << "WebSocket not connected!";
        return;
    }

    QJsonObject json;
    json["type"] = "recall_message";
    json["messageId"] = messageId;
    json["recipientId"] = recipientId;

    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "Recall message sent via WebSocket:" << json;
}

void WebSocketClient::onConnected()
{
    qDebug() << "WebSocket connected!";
    
    m_state = ConnectionState::Connected;
    resetReconnectAttempts();
    startHeartbeat();
    flushMessageQueue();
    
    if (m_reconnectAttempts > 0) {
        emit reconnected();
    }
    
    emit connected();
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "WebSocket disconnected.";
    
    stopHeartbeat();
    
    if (m_state == ConnectionState::Connected && m_reconnectConfig.autoReconnect) {
        m_state = ConnectionState::Reconnecting;
        scheduleReconnect();
    } else {
        m_state = ConnectionState::Disconnected;
    }
    
    emit disconnected();
}

void WebSocketClient::onTextMessageReceived(const QString& message)
{
    qDebug() << "WebSocket message received:" << message;
    
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON message received";
        return;
    }

    QJsonObject json = doc.object();
    QString type = json["type"].toString();

    if (type == "connected") {
        qDebug() << "WebSocket connection confirmed for user:" << json["userId"].toInt();
    }
    else if (type == "new_message") {
        handleNewMessage(json["message"].toObject());
    }
    else if (type == "message_sent") {
        emit messageSent(json["message"].toObject());
    }
    else if (type == "message_error") {
        emit messageError(json["message"].toString());
    }
    else if (type == "friend_request") {
        handleFriendRequest(json["data"].toObject());
    }
    else if (type == "friend_request_accepted") {
        handleFriendRequestAccepted(json["data"].toObject());
    }
    else if (type == "friend_deleted") {
        handleFriendDeleted(json["data"].toObject());
    }
    else if (type == "user_status") {
        handleUserStatus(json);
    }
    else if (type == "messages_read") {
        int recipientId = json["recipientId"].toInt();
        qDebug() << "Messages read by recipient:" << recipientId;
        emit messagesRead(recipientId);
    }
    else if (type == "message_recalled") {
        int messageId = json["messageId"].toInt();
        qDebug() << "Message recalled:" << messageId;
        emit messageRecalled(messageId);
    }
    else if (type == "auth_error") {
        emit errorOccurred(json["message"].toString());
    }
    else if (type == "pong") {
        handlePong();
    }
    else {
        qDebug() << "Unknown message type:" << type;
    }
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QString errorString = m_socket.errorString();
    qWarning() << "WebSocket error:" << errorString;
    emit errorOccurred(errorString);
}

void WebSocketClient::startHeartbeat()
{
    if (!m_heartbeatConfig.enabled) {
        return;
    }
    
    m_missedHeartbeats = 0;
    
    if (!m_heartbeatTimer) {
        m_heartbeatTimer = new QTimer(this);
        connect(m_heartbeatTimer, &QTimer::timeout, this, &WebSocketClient::onHeartbeatTimer);
    }
    
    m_heartbeatTimer->start(m_heartbeatConfig.intervalMs);
    qDebug() << "Heartbeat started, interval:" << m_heartbeatConfig.intervalMs << "ms";
}

void WebSocketClient::stopHeartbeat()
{
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
    }
    if (m_heartbeatTimeoutTimer) {
        m_heartbeatTimeoutTimer->stop();
    }
    m_missedHeartbeats = 0;
    qDebug() << "Heartbeat stopped";
}

void WebSocketClient::sendPing()
{
    if (!isConnected()) {
        return;
    }
    
    QJsonObject json;
    json["type"] = "ping";
    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "Ping sent";
}

void WebSocketClient::handlePong()
{
    m_missedHeartbeats = 0;
    if (m_heartbeatTimeoutTimer) {
        m_heartbeatTimeoutTimer->stop();
    }
    qDebug() << "Pong received, heartbeat ok";
}

void WebSocketClient::onHeartbeatTimer()
{
    if (!isConnected()) {
        return;
    }
    
    sendPing();
    
    if (!m_heartbeatTimeoutTimer) {
        m_heartbeatTimeoutTimer = new QTimer(this);
        m_heartbeatTimeoutTimer->setSingleShot(true);
        connect(m_heartbeatTimeoutTimer, &QTimer::timeout, this, &WebSocketClient::onHeartbeatTimeout);
    }
    
    m_heartbeatTimeoutTimer->start(m_heartbeatConfig.timeoutMs);
}

void WebSocketClient::onHeartbeatTimeout()
{
    m_missedHeartbeats++;
    m_heartbeatTimeoutTimer->stop();
    
    qDebug() << "Heartbeat timeout, missed:" << m_missedHeartbeats 
             << "/" << m_heartbeatConfig.maxMissedHeartbeats;
    
    if (m_missedHeartbeats >= m_heartbeatConfig.maxMissedHeartbeats) {
        qWarning() << "Max missed heartbeats reached, reconnecting...";
        m_socket.close();
        scheduleReconnect();
    }
}

void WebSocketClient::scheduleReconnect()
{
    if (m_reconnectConfig.maxReconnectAttempts > 0 &&
        m_reconnectAttempts >= m_reconnectConfig.maxReconnectAttempts) {
        qWarning() << "Max reconnect attempts reached";
        m_state = ConnectionState::Failed;
        emit connectionFailed();
        return;
    }
    
    int delay = calculateReconnectDelay();
    m_reconnectAttempts++;
    
    qDebug() << "Scheduling reconnect attempt" << m_reconnectAttempts 
             << "in" << delay << "ms";
    
    emit reconnecting(m_reconnectAttempts, delay);
    
    if (!m_reconnectTimer) {
        m_reconnectTimer = new QTimer(this);
        m_reconnectTimer->setSingleShot(true);
        connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::onReconnectTimer);
    }
    
    m_reconnectTimer->start(delay);
}

void WebSocketClient::onReconnectTimer()
{
    attemptReconnect();
}

void WebSocketClient::attemptReconnect()
{
    if (m_token.isEmpty()) {
        qWarning() << "No token available for reconnect";
        m_state = ConnectionState::Failed;
        emit connectionFailed();
        return;
    }
    
    qDebug() << "Attempting to reconnect...";
    m_state = ConnectionState::Reconnecting;
    connectToServer(m_token);
}

void WebSocketClient::resetReconnectAttempts()
{
    m_reconnectAttempts = 0;
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
    }
}

int WebSocketClient::calculateReconnectDelay() const
{
    double delay = m_reconnectConfig.baseDelayMs * 
                   qPow(m_reconnectConfig.backoffMultiplier, m_reconnectAttempts);
    return qMin(static_cast<int>(delay), m_reconnectConfig.maxDelayMs);
}

void WebSocketClient::flushMessageQueue()
{
    if (m_messageQueue.isEmpty()) {
        return;
    }
    
    qDebug() << "Flushing message queue, size:" << m_messageQueue.size();
    
    while (!m_messageQueue.isEmpty()) {
        QueuedMessage msg = m_messageQueue.dequeue();
        QJsonDocument doc(msg.message);
        m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        qDebug() << "Sent queued message, type:" << msg.message["type"].toString();
    }
}

void WebSocketClient::handleNewMessage(const QJsonObject& data)
{
    emit messageReceived(data);
}

void WebSocketClient::handleFriendRequest(const QJsonObject& data)
{
    int userId = data["user_id"].toInt();
    QString username = data["username"].toString();
    QString note = data["note"].toString();
    
    qDebug() << "Friend request received from:" << username;
    emit friendRequestReceived(userId, username, note);
}

void WebSocketClient::handleFriendRequestAccepted(const QJsonObject& data)
{
    int friendId = data["friend_id"].toInt();
    QString username = data["username"].toString();
    
    qDebug() << "Friend request accepted by:" << username;
    emit friendRequestAccepted(friendId, username);
}

void WebSocketClient::handleFriendDeleted(const QJsonObject& data)
{
    int friendId = data["friend_id"].toInt();
    
    qDebug() << "Friend deleted notification received, friend ID:" << friendId;
    emit friendDeleted(friendId);
}

void WebSocketClient::handleUserStatus(const QJsonObject& data)
{
    int userId = data["userId"].toInt();
    bool isOnline = data["isOnline"].toBool();
    
    qDebug() << "User status changed, user ID:" << userId << "online:" << isOnline;
    emit userStatusChanged(userId, isOnline);
}
