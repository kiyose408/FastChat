#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>

class WebSocketClient : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketClient(QObject *parent = nullptr);
    
    void connectToServer(const QString& token);
    void disconnectFromServer();
    bool isConnected() const;
    
    void sendMessage(int recipientId, const QString& text);
    void sendFileMessage(int recipientId, const QString& fileUrl, const QString& fileName, const QString& messageType);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    
    void messageReceived(const QJsonObject& message);
    void messageSent(const QJsonObject& message);
    void messageError(const QString& error);
    
    void friendRequestReceived(int userId, const QString& username, const QString& note);
    void friendRequestAccepted(int friendId, const QString& username);
    void friendDeleted(int friendId);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket m_socket;
    QString m_token;
    
    void handleNewMessage(const QJsonObject& data);
    void handleFriendRequest(const QJsonObject& data);
    void handleFriendRequestAccepted(const QJsonObject& data);
    void handleFriendDeleted(const QJsonObject& data);
};

#endif // WEBSOCKETCLIENT_H
