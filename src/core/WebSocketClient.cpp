#include "WebSocketClient.h" // 包含头文件
#include <QUrl>             // 用于处理 URL
#include <QJsonDocument>    // 用于解析和生成 JSON 文档
#include <QJsonObject>      // 用于处理 JSON 对象
#include <QDateTime>        // 用于处理日期和时间
#include <QDebug>           // 用于调试输出

// 构造函数
// 初始化 WebSocketClient 对象，设置信号与槽的连接
WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent) // 调用基类 QObject 的构造函数
{
    // 连接 QWebSocket 的 "connected" 信号到本类的 onConnected slot
    // 当 WebSocket 连接成功建立时，会触发 onConnected 函数
    connect(&m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);

    // 连接 QWebSocket 的 "textMessageReceived" 信号到本类的 onTextMessageReceived slot
    // 当从服务器接收到文本消息时，会触发 onTextMessageReceived 函数
    connect(&m_socket, &QWebSocket::textMessageReceived,
            this, &WebSocketClient::onTextMessageReceived);
}

// 连接到 WebSocket 服务器
// 参数: token - 用于身份验证的令牌
void WebSocketClient::connectToServer(const QString &token)
{
    // 构造 WebSocket URL，包含查询参数 token
    // 注意：这里的 URL 是示例地址 "ws://localhost:3000/websocket"
    // 实际使用时应替换为你的服务器地址
    QUrl url("ws://localhost:3000/websocket?token=" + token);

    // 使用 QWebSocket 打开连接到指定的 URL
    m_socket.open(url);
}

// 当 WebSocket 连接成功建立时调用
// 这是连接信号的槽函数
void WebSocketClient::onConnected()
{
    // 输出调试信息，表示连接已建立
    qDebug() << "WebSocket connected!";

    // 发射自定义信号 connectionEstablished，通知外界连接已成功
    emit connectionEstablished();
}

// 当从 WebSocket 服务器接收到文本消息时调用
// 这是接收消息信号的槽函数
// 参数: message - 从服务器接收到的原始文本消息 (JSON 格式字符串)
void WebSocketClient::onTextMessageReceived(const QString &message)
{
    // 将接收到的 JSON 字符串转换为 QJsonDocument 对象
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    // 检查解析是否成功（即是否为有效的 JSON 对象）
    if (!doc.isObject()) {
        // 如果不是有效对象，直接返回，不处理
        return;
    }

    // 获取 JSON 文档中的根对象
    QJsonObject json = doc.object();

    // 从 JSON 对象中提取 "type" 字段，用于判断消息类型
    QString type = json["type"].toString();

    // 根据消息类型进行不同处理
    if (type == "new_message") {
        // 处理新消息类型
        // 从 JSON 对象中提取 "message" 字段（它本身是一个对象）
        QJsonObject msg = json["message"].toObject();

        // 提取消息的发送者 ID
        int senderId = msg["sender_id"].toInt();

        // 提取消息内容
        QString content = msg["content"].toString();

        // 提取时间戳，并截取前 19 个字符（假设格式为 "YYYY-MM-DDTHH:mm:ss"）
        QString time = msg["created_at"].toString().left(19);

        // 发射自定义信号 messageReceived，传递解析后的消息数据
        emit messageReceived(senderId, content, time);
    } else if (type == "auth_error") {
        // 处理认证错误类型
        // 从 JSON 对象中提取 "message" 字段作为错误描述
        emit errorOccurred(json["message"].toString());
    }
    // 可在此处添加更多 else if 分支来处理其他类型的服务器消息
    // 例如：用户上线/下线通知、系统公告等
}

// 发送文本消息给指定的接收者
// 参数: recipientId - 接收者的 ID
//       text - 要发送的消息内容
void WebSocketClient::sendMessage(int recipientId, const QString &text)
{
    // 检查 WebSocket 连接状态是否为已连接
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        // 如果未连接，输出警告信息
        qWarning() << "WebSocket not connected!";
        // 返回，不执行发送操作
        return;
    }

    // 创建用于发送的 JSON 对象
    QJsonObject json;
    // 设置消息类型为 "send_message"
    json["type"] = "send_message";
    // 设置接收者 ID
    json["recipientId"] = recipientId;
    // 设置消息内容
    json["content"] = text;

    // 将 JSON 对象转换为 QJsonDocument
    QJsonDocument doc(json);

    // 将 JSON 文档序列化为紧凑格式的 UTF-8 字节串
    // 然后转换为 QString 并通过 WebSocket 发送
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}
