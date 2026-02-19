# ApiService 重试机制与 WebSocket 重连优化实施计划书

## 项目信息

| 项目 | 内容 |
|------|------|
| 项目名称 | 网络可靠性优化 |
| 版本 | v0.12 |
| 负责人 | 开发团队 |
| 开始日期 | 2026年2月 |
| 预计工期 | 7个工作日 |

---

## 一、项目背景

### 1.1 问题描述

当前 FastChat 客户端在网络不稳定情况下存在以下问题：

1. **HTTP 请求无重试**：网络波动导致请求直接失败
2. **WebSocket 无心跳**：无法检测连接存活状态
3. **WebSocket 无重连**：断线后需手动重新登录
4. **消息丢失**：断线期间发送的消息无法送达

### 1.2 目标

- HTTP 请求重试成功率 > 95%
- WebSocket 自动重连成功率 > 99%
- 断线期间消息不丢失
- 用户无感知的网络恢复

---

## 二、详细实施计划

### 2.1 第一阶段：RetryPolicy 类实现

**预计工时：4小时**

#### 2.1.1 创建文件

```
src/core/RetryPolicy.h
src/core/RetryPolicy.cpp
```

#### 2.1.2 RetryPolicy.h

```cpp
#ifndef RETRYPOLICY_H
#define RETRYPOLICY_H

#include <QObject>
#include <QNetworkReply>
#include <QList>

struct RetryConfig {
    int maxRetries = 3;
    int baseDelayMs = 1000;
    int maxDelayMs = 10000;
    double backoffMultiplier = 2.0;
    QList<int> retryableStatusCodes;
    
    RetryConfig() {
        retryableStatusCodes = {408, 429, 500, 502, 503, 504};
    }
};

class RetryPolicy : public QObject
{
    Q_OBJECT

public:
    explicit RetryPolicy(const RetryConfig& config = RetryConfig(), QObject* parent = nullptr);
    
    bool shouldRetry(QNetworkReply::NetworkError error, int statusCode) const;
    int calculateDelay(int retryCount) const;
    bool canRetry(int retryCount) const;
    bool isRetryableStatus(int statusCode) const;
    bool isRetryableError(QNetworkReply::NetworkError error) const;
    
    void setConfig(const RetryConfig& config);
    RetryConfig config() const;

private:
    RetryConfig m_config;
};

#endif // RETRYPOLICY_H
```

#### 2.1.3 RetryPolicy.cpp

```cpp
#include "RetryPolicy.h"
#include <QDebug>
#include <QtMath>

RetryPolicy::RetryPolicy(const RetryConfig& config, QObject* parent)
    : QObject(parent)
    , m_config(config)
{
}

bool RetryPolicy::shouldRetry(QNetworkReply::NetworkError error, int statusCode) const
{
    if (!canRetry(0)) {
        return false;
    }
    
    if (isRetryableError(error)) {
        return true;
    }
    
    if (isRetryableStatus(statusCode)) {
        return true;
    }
    
    return false;
}

int RetryPolicy::calculateDelay(int retryCount) const
{
    if (retryCount <= 0) {
        return m_config.baseDelayMs;
    }
    
    double delay = m_config.baseDelayMs * 
                   qPow(m_config.backoffMultiplier, retryCount - 1);
    
    return qMin(static_cast<int>(delay), m_config.maxDelayMs);
}

bool RetryPolicy::canRetry(int retryCount) const
{
    return retryCount < m_config.maxRetries;
}

bool RetryPolicy::isRetryableStatus(int statusCode) const
{
    return m_config.retryableStatusCodes.contains(statusCode);
}

bool RetryPolicy::isRetryableError(QNetworkReply::NetworkError error) const
{
    switch (error) {
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::TemporaryNetworkFailureError:
        case QNetworkReply::NetworkSessionFailedError:
        case QNetworkReply::UnknownNetworkError:
        case QNetworkReply::ServiceUnavailableError:
            return true;
        default:
            return false;
    }
}

void RetryPolicy::setConfig(const RetryConfig& config)
{
    m_config = config;
}

RetryConfig RetryPolicy::config() const
{
    return m_config;
}
```

---

### 2.2 第二阶段：ApiService 重试集成

**预计工时：6小时**

#### 2.2.1 修改 ApiService.h

添加以下内容：

```cpp
#include "RetryPolicy.h"
#include <QTimer>
#include <QUuid>

struct PendingRequest {
    QString id;
    QNetworkRequest request;
    QByteArray data;
    QString method;
    int retryCount = 0;
    std::function<void(QNetworkReply*)> originalCallback;
};

class ApiService : public QObject
{
    // ... 现有代码 ...

public:
    void setRetryConfig(const RetryConfig& config);
    void setRequestTimeout(int ms);
    
private:
    RetryPolicy* m_retryPolicy;
    int m_timeoutMs = 30000;
    QMap<QString, PendingRequest> m_pendingRequests;
    
    QString generateRequestId();
    void executeWithRetry(const QString& method, const QUrl& url, 
                          const QByteArray& data,
                          std::function<void(QNetworkReply*)> callback);
    void retryRequest(const QString& requestId);
    void handleRetryTimeout(const QString& requestId);
};
```

#### 2.2.2 修改 ApiService.cpp

添加重试逻辑：

```cpp
void ApiService::executeWithRetry(const QString& method, const QUrl& url, 
                                   const QByteArray& data,
                                   std::function<void(QNetworkReply*)> callback)
{
    QString requestId = generateRequestId();
    
    PendingRequest req;
    req.id = requestId;
    req.request = QNetworkRequest(url);
    req.request.setRawHeader("Authorization", 
        ("Bearer " + SessionManager::instance().token()).toUtf8());
    req.data = data;
    req.method = method;
    req.retryCount = 0;
    req.originalCallback = callback;
    
    m_pendingRequests[requestId] = req;
    
    executeRequest(requestId);
}

void ApiService::executeRequest(const QString& requestId)
{
    if (!m_pendingRequests.contains(requestId)) {
        return;
    }
    
    PendingRequest& req = m_pendingRequests[requestId];
    
    QNetworkReply* reply = nullptr;
    
    if (req.method == "GET") {
        reply = m_netManager.get(req.request);
    } else if (req.method == "POST") {
        reply = m_netManager.post(req.request, req.data);
    } else if (req.method == "PUT") {
        reply = m_netManager.put(req.request, req.data);
    } else if (req.method == "DELETE") {
        reply = m_netManager.deleteResource(req.request);
    }
    
    if (reply) {
        QTimer::singleShot(m_timeoutMs, this, [this, requestId]() {
            handleRetryTimeout(requestId);
        });
        
        connect(reply, &QNetworkReply::finished, this, [this, requestId]() {
            handleReplyFinished(requestId);
        });
    }
}

void ApiService::handleReplyFinished(const QString& requestId)
{
    if (!m_pendingRequests.contains(requestId)) {
        return;
    }
    
    PendingRequest& req = m_pendingRequests[requestId];
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    
    int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError error = reply->error();
    
    if (error != QNetworkReply::NoError) {
        if (m_retryPolicy->shouldRetry(error, statusCode) && 
            m_retryPolicy->canRetry(req.retryCount)) {
            
            int delay = m_retryPolicy->calculateDelay(req.retryCount);
            req.retryCount++;
            
            qDebug() << "Retrying request" << requestId 
                     << "attempt" << req.retryCount 
                     << "after" << delay << "ms";
            
            QTimer::singleShot(delay, this, [this, requestId]() {
                executeRequest(requestId);
            });
            
            reply->deleteLater();
            return;
        }
    }
    
    req.originalCallback(reply);
    m_pendingRequests.remove(requestId);
    reply->deleteLater();
}

void ApiService::handleRetryTimeout(const QString& requestId)
{
    if (m_pendingRequests.contains(requestId)) {
        qDebug() << "Request timeout:" << requestId;
        m_pendingRequests.remove(requestId);
    }
}
```

---

### 2.3 第三阶段：WebSocket 心跳机制

**预计工时：4小时**

#### 2.3.1 修改 WebSocketClient.h

```cpp
struct HeartbeatConfig {
    bool enabled = true;
    int intervalMs = 30000;
    int timeoutMs = 10000;
    int maxMissedHeartbeats = 3;
};

class WebSocketClient : public QObject
{
    // ... 现有代码 ...

public:
    void setHeartbeatConfig(const HeartbeatConfig& config);
    void startHeartbeat();
    void stopHeartbeat();

private slots:
    void onHeartbeatTimer();
    void onHeartbeatTimeout();

private:
    HeartbeatConfig m_heartbeatConfig;
    QTimer* m_heartbeatTimer;
    QTimer* m_heartbeatTimeoutTimer;
    int m_missedHeartbeats = 0;
    
    void sendPing();
    void handlePong();
};
```

#### 2.3.2 添加心跳实现

```cpp
void WebSocketClient::startHeartbeat()
{
    if (!m_heartbeatConfig.enabled) {
        return;
    }
    
    m_missedHeartbeats = 0;
    
    if (!m_heartbeatTimer) {
        m_heartbeatTimer = new QTimer(this);
        connect(m_heartbeatTimer, &QTimer::timeout, 
                this, &WebSocketClient::onHeartbeatTimer);
    }
    
    m_heartbeatTimer->start(m_heartbeatConfig.intervalMs);
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
}

void WebSocketClient::onHeartbeatTimer()
{
    if (!isConnected()) {
        return;
    }
    
    sendPing();
    
    if (!m_heartbeatTimeoutTimer) {
        m_heartbeatTimeoutTimer = new QTimer(this);
        connect(m_heartbeatTimeoutTimer, &QTimer::timeout,
                this, &WebSocketClient::onHeartbeatTimeout);
    }
    
    m_heartbeatTimeoutTimer->start(m_heartbeatConfig.timeoutMs);
}

void WebSocketClient::onHeartbeatTimeout()
{
    m_missedHeartbeats++;
    m_heartbeatTimeoutTimer->stop();
    
    qDebug() << "Heartbeat timeout, missed:" << m_missedHeartbeats;
    
    if (m_missedHeartbeats >= m_heartbeatConfig.maxMissedHeartbeats) {
        qWarning() << "Max missed heartbeats reached, reconnecting...";
        m_socket.close();
        scheduleReconnect();
    }
}

void WebSocketClient::sendPing()
{
    QJsonObject json;
    json["type"] = "ping";
    QJsonDocument doc(json);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void WebSocketClient::handlePong()
{
    m_missedHeartbeats = 0;
    if (m_heartbeatTimeoutTimer) {
        m_heartbeatTimeoutTimer->stop();
    }
}
```

---

### 2.4 第四阶段：WebSocket 重连机制

**预计工时：6小时**

#### 2.4.1 修改 WebSocketClient.h

```cpp
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Failed
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
    // ... 现有代码 ...

public:
    void setReconnectConfig(const ReconnectConfig& config);
    ConnectionState connectionState() const;
    
signals:
    void reconnecting(int attempt, int delay);
    void reconnected();
    void connectionFailed();

private slots:
    void onReconnectTimer();

private:
    ReconnectConfig m_reconnectConfig;
    int m_reconnectAttempts = 0;
    QTimer* m_reconnectTimer;
    ConnectionState m_state = ConnectionState::Disconnected;
    
    void scheduleReconnect();
    void attemptReconnect();
    void resetReconnectAttempts();
    int calculateReconnectDelay() const;
};
```

#### 2.4.2 添加重连实现

```cpp
void WebSocketClient::onDisconnected()
{
    qDebug() << "WebSocket disconnected.";
    
    stopHeartbeat();
    
    if (m_state == ConnectionState::Connected && 
        m_reconnectConfig.autoReconnect) {
        m_state = ConnectionState::Reconnecting;
        scheduleReconnect();
    } else {
        m_state = ConnectionState::Disconnected;
    }
    
    emit disconnected();
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
        connect(m_reconnectTimer, &QTimer::timeout,
                this, &WebSocketClient::onReconnectTimer);
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
        return;
    }
    
    qDebug() << "Attempting to reconnect...";
    m_state = ConnectionState::Reconnecting;
    connectToServer(m_token);
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
```

---

### 2.5 第五阶段：消息队列机制

**预计工时：4小时**

#### 2.5.1 创建 MessageQueue 类

```cpp
// MessageQueue.h
#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QObject>
#include <QJsonObject>
#include <QList>

struct QueuedMessage {
    QString id;
    QJsonObject message;
    qint64 timestamp;
    int retryCount = 0;
};

class MessageQueue : public QObject
{
    Q_OBJECT

public:
    explicit MessageQueue(int maxSize = 100, QObject* parent = nullptr);
    
    void enqueue(const QJsonObject& message);
    QueuedMessage dequeue();
    void ackReceived(const QString& messageId);
    void clear();
    int size() const;
    bool isEmpty() const;
    void setMaxSize(int size);

private:
    QList<QueuedMessage> m_queue;
    int m_maxSize;
};

#endif // MESSAGEQUEUE_H
```

#### 2.5.2 集成到 WebSocketClient

```cpp
void WebSocketClient::sendMessage(int recipientId, const QString& text)
{
    QJsonObject json;
    json["type"] = "send_message";
    json["recipientId"] = recipientId;
    json["content"] = text;
    json["messageId"] = QUuid::createUuid().toString();

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

void WebSocketClient::flushMessageQueue()
{
    qDebug() << "Flushing message queue, size:" << m_messageQueue.size();
    
    while (!m_messageQueue.isEmpty()) {
        QueuedMessage msg = m_messageQueue.dequeue();
        QJsonDocument doc(msg.message);
        m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        qDebug() << "Sent queued message:" << msg.id;
    }
}
```

---

## 三、后端配合修改

### 3.1 添加心跳响应

修改 `src/config/websocket.js`：

```javascript
// 处理心跳
wsConnection.on('message', (data) => {
    try {
        const message = JSON.parse(data.toString());
        
        if (message.type === 'ping') {
            wsConnection.send(JSON.stringify({ type: 'pong' }));
            return;
        }
        
        // ... 其他消息处理
    } catch (err) {
        console.error('Message parse error:', err);
    }
});
```

---

## 四、UI 配合修改

### 4.1 显示重连状态

在 ChatMainWindow 中添加：

```cpp
void ChatMainWindow::onWebSocketReconnecting(int attempt, int delay)
{
    ui->connectionStatusLabel->setText(
        QString::fromUtf8("正在重连... (%1次，%2秒后)").arg(attempt).arg(delay/1000));
}

void ChatMainWindow::onWebSocketReconnected()
{
    ui->connectionStatusLabel->setText(QString::fromUtf8("已连接"));
}

void ChatMainWindow::onWebSocketConnectionFailed()
{
    ui->connectionStatusLabel->setText(QString::fromUtf8("连接失败"));
}
```

---

## 五、测试计划

### 5.1 单元测试

| 测试项 | 测试内容 |
|--------|----------|
| RetryPolicy | 重试条件判断 |
| RetryPolicy | 延迟计算正确性 |
| MessageQueue | 入队出队操作 |
| MessageQueue | 队列大小限制 |

### 5.2 集成测试

| 测试项 | 测试内容 |
|--------|----------|
| HTTP重试 | 模拟服务器错误，验证重试 |
| HTTP超时 | 模拟超时，验证取消 |
| WebSocket重连 | 断开网络，验证自动重连 |
| 心跳检测 | 停止心跳响应，验证重连 |
| 消息队列 | 断线发送消息，重连后验证送达 |

### 5.3 压力测试

| 测试项 | 测试内容 |
|--------|----------|
| 并发请求 | 100个并发请求，验证重试不冲突 |
| 消息洪泛 | 快速发送1000条消息，验证队列处理 |
| 长时间运行 | 24小时运行，验证无内存泄漏 |

---

## 六、里程碑

| 里程碑 | 日期 | 交付物 |
|--------|------|--------|
| M1 | Day 2 | RetryPolicy 类完成 |
| M2 | Day 3 | ApiService 重试集成完成 |
| M3 | Day 4 | WebSocket 心跳完成 |
| M4 | Day 6 | WebSocket 重连完成 |
| M5 | Day 7 | 消息队列完成，测试通过 |

---

## 七、验收标准

1. HTTP 请求在网络波动时自动重试，成功率 > 95%
2. WebSocket 断线后 30 秒内自动重连
3. 心跳超时后自动触发重连
4. 断线期间发送的消息在重连后自动发送
5. 无内存泄漏
6. 单元测试覆盖率 > 80%

---

*文档版本：v1.0*
*最后更新：2026年2月*
