#include "WebSocketClient.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent)
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
    QUrl url("ws://localhost:3000/websocket?token=" + token);
    qDebug() << "Connecting to WebSocket:" << url.toString();
    m_socket.open(url);
}

void WebSocketClient::disconnectFromServer()
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        m_socket.close();
    }
}

bool WebSocketClient::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::sendMessage(int recipientId, const QString& text)
{
    if (!isConnected()) {
        qWarning() << "WebSocket not connected!";
        emit messageError("WebSocket not connected.");
        return;
    }

    QJsonObject json;
    json["type"] = "send_message";
    json["recipientId"] = recipientId;
    json["content"] = text;

    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "Message sent via WebSocket:" << json;
}

void WebSocketClient::sendFileMessage(int recipientId, const QString& fileUrl, const QString& fileName, const QString& messageType)
{
    if (!isConnected()) {
        qWarning() << "WebSocket not connected!";
        emit messageError("WebSocket not connected.");
        return;
    }

    QJsonObject json;
    json["type"] = "send_file";
    json["recipientId"] = recipientId;
    json["fileUrl"] = fileUrl;
    json["fileName"] = fileName;
    json["messageType"] = messageType;

    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    qDebug() << "File message sent via WebSocket:" << json;
}

void WebSocketClient::onConnected()
{
    qDebug() << "WebSocket connected!";
    emit connected();
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "WebSocket disconnected.";
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
    else if (type == "auth_error") {
        emit errorOccurred(json["message"].toString());
    }
    else if (type == "pong") {
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
