#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QTimer>
#include "MessageQueue.h"
#include "AppConfig.h"

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Failed
};

struct HeartbeatConfig {
    bool enabled = true;
    int intervalMs = 30000;
    int timeoutMs = 10000;
    int maxMissedHeartbeats = 3;
};

struct ReconnectConfig {
    bool autoReconnect = true;
    int maxReconnectAttempts = 10;
    int baseDelayMs = 1000;
    int maxDelayMs = 30000;
    double backoffMultiplier = 1.5;
};

class WebSocketClient : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketClient(QObject *parent = nullptr);
    
    void connectToServer(const QString& token);
    void disconnectFromServer();
    bool isConnected() const;
    ConnectionState connectionState() const;
    
    void sendMessage(int recipientId, const QString& text);
    void sendFileMessage(int recipientId, const QString& fileUrl, const QString& fileName, const QString& messageType);
    void markMessagesAsRead(int senderId);
    void recallMessage(int messageId, int recipientId);
    
    void setHeartbeatConfig(const HeartbeatConfig& config);
    void setReconnectConfig(const ReconnectConfig& config);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void reconnecting(int attempt, int delay);
    void reconnected();
    void connectionFailed();
    
    void messageReceived(const QJsonObject& message);
    void messageSent(const QJsonObject& message);
    void messageError(const QString& error);
    void messageRecalled(int messageId);
    
    void friendRequestReceived(int userId, const QString& username, const QString& note);
    void friendRequestAccepted(int friendId, const QString& username);
    void friendDeleted(int friendId);
    
    void userStatusChanged(int userId, bool isOnline);
    void messagesRead(int recipientId);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);
    void onHeartbeatTimer();
    void onHeartbeatTimeout();
    void onReconnectTimer();

private:
    QWebSocket m_socket;
    QString m_token;
    ConnectionState m_state = ConnectionState::Disconnected;
    
    HeartbeatConfig m_heartbeatConfig;
    QTimer* m_heartbeatTimer = nullptr;
    QTimer* m_heartbeatTimeoutTimer = nullptr;
    int m_missedHeartbeats = 0;
    
    ReconnectConfig m_reconnectConfig;
    int m_reconnectAttempts = 0;
    QTimer* m_reconnectTimer = nullptr;
    
    MessageQueue m_messageQueue;
    
    void startHeartbeat();
    void stopHeartbeat();
    void sendPing();
    void handlePong();
    
    void scheduleReconnect();
    void attemptReconnect();
    void resetReconnectAttempts();
    int calculateReconnectDelay() const;
    
    void flushMessageQueue();
    
    void handleNewMessage(const QJsonObject& data);
    void handleFriendRequest(const QJsonObject& data);
    void handleFriendRequestAccepted(const QJsonObject& data);
    void handleFriendDeleted(const QJsonObject& data);
    void handleUserStatus(const QJsonObject& data);
};

#endif // WEBSOCKETCLIENT_H
