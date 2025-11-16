#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

// 包含必要的 Qt 核心和网络模块头文件
#include <QObject> // QObject 是所有 Qt 对象的基类
#include <QWebSocket> // 包含 QWebSocket 类，用于 WebSocket 连接

// 声明 WebSocketClient 类，继承自 QObject
class WebSocketClient : public QObject
{
    Q_OBJECT // 宏用于启用 Qt 的元对象系统 (MOC)，支持信号和槽

public:
    // 构造函数，接受一个可选的父对象指针
    explicit WebSocketClient(QObject *parent = nullptr);
    // 提供一个方法来连接到 WebSocket 服务器，参数为 token
    void connectToServer(const QString& token);

    // 定义信号 (Signals) - 用于对象间通信
signals:
    // 当收到消息时发射此信号，参数包括发送者 ID、消息文本和时间戳
    void messageReceived(int senderId, const QString& text, const QString& time);
    // 当与服务器的连接成功建立时发射此信号
    void connectionEstablished();
    // 当发生错误时发射此信号，参数为错误描述
    void errorOccurred(const QString& error);

    // 定义公有槽函数 (Public Slots) - 可以被信号调用或外部调用
public slots:
    // 发送消息给指定的接收者
    void sendMessage(int recipientId, const QString& text);

    // 定义私有槽函数 (Private Slots) - 只能在类内部使用
private slots:
    // 当与服务器连接成功时调用的槽函数
    void onConnected();
    // 当从服务器接收到文本消息时调用的槽函数
    void onTextMessageReceived(const QString& message);

private:
    // 私有成员变量：存储实际的 WebSocket 连接对象
    QWebSocket m_socket; // 使用 QWebSocket 实现底层 WebSocket 功能
};

#endif // WEBSOCKETCLIENT_H
