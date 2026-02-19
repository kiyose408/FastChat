# ApiService 请求重试机制与 WebSocket 断线重连优化技术方案

## 文档信息

| 项目 | 内容 |
|------|------|
| 版本 | v1.0 |
| 日期 | 2026年2月 |
| 状态 | 设计阶段 |

---

## 一、现状分析

### 1.1 ApiService 当前问题

| 问题 | 影响 | 严重程度 |
|------|------|----------|
| 无重试机制 | 网络波动导致请求失败，用户体验差 | 高 |
| 无超时控制 | 请求可能无限等待 | 中 |
| 无错误分类 | 所有错误统一处理，无法针对性恢复 | 中 |
| 无请求队列 | 并发请求管理混乱 | 低 |

### 1.2 WebSocketClient 当前问题

| 问题 | 影响 | 严重程度 |
|------|------|----------|
| 无自动重连 | 断线后需手动重连，消息丢失 | 高 |
| 无心跳机制 | 无法检测连接存活状态 | 高 |
| 无消息队列 | 断线期间消息丢失 | 高 |
| 无重连退避 | 频繁重连可能造成服务器压力 | 中 |

---

## 二、技术方案设计

### 2.1 ApiService 请求重试机制

#### 2.1.1 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                      ApiService                              │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ RetryPolicy │  │ TimeoutMgr  │  │   RequestQueue      │  │
│  │  - maxRetry │  │  - timeout  │  │   - pendingReqs     │  │
│  │  - backoff  │  │  - timer    │  │   - priority        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐    │
│  │              NetworkAccessManager                    │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

#### 2.1.2 重试策略配置

```cpp
struct RetryConfig {
    int maxRetries = 3;           // 最大重试次数
    int baseDelayMs = 1000;       // 基础延迟（毫秒）
    int maxDelayMs = 10000;       // 最大延迟（毫秒）
    double backoffMultiplier = 2.0; // 退避乘数
    QList<int> retryableStatusCodes = {408, 429, 500, 502, 503, 504};
};
```

#### 2.1.3 指数退避算法

```
重试延迟 = min(baseDelay * (backoffMultiplier ^ retryCount), maxDelay)

示例（baseDelay=1000ms, multiplier=2.0）:
- 第1次重试: 1000ms
- 第2次重试: 2000ms
- 第3次重试: 4000ms
```

#### 2.1.4 错误分类处理

| 错误类型 | HTTP状态码 | 处理策略 |
|----------|------------|----------|
| 网络错误 | - | 自动重试 |
| 超时错误 | 408 | 自动重试 |
| 服务端错误 | 500, 502, 503, 504 | 自动重试 |
| 限流错误 | 429 | 延迟重试 |
| 认证错误 | 401 | 不重试，触发重新登录 |
| 权限错误 | 403 | 不重试 |
| 资源不存在 | 404 | 不重试 |
| 客户端错误 | 400, 422 | 不重试 |

#### 2.1.5 核心类设计

```cpp
// RetryPolicy.h
class RetryPolicy : public QObject {
    Q_OBJECT
public:
    explicit RetryPolicy(const RetryConfig& config, QObject* parent = nullptr);
    
    bool shouldRetry(QNetworkReply::NetworkError error, int statusCode);
    int calculateDelay(int retryCount);
    bool canRetry(int retryCount);
    
private:
    RetryConfig m_config;
};

// ApiRequest.h
struct ApiRequest {
    QString id;
    QNetworkRequest request;
    QByteArray data;
    QString method;
    int retryCount = 0;
    int priority = 0;
    std::function<void(QNetworkReply*)> callback;
};

// ApiService 改进
class ApiService : public QObject {
    Q_OBJECT
public:
    void setRetryConfig(const RetryConfig& config);
    void setTimeout(int ms);
    
private:
    void executeRequest(ApiRequest& req);
    void onReplyFinished(QNetworkReply* reply, ApiRequest& req);
    void retryRequest(ApiRequest& req);
    
    RetryPolicy* m_retryPolicy;
    QTimer* m_timeoutTimer;
    QMap<QString, ApiRequest> m_pendingRequests;
};
```

---

### 2.2 WebSocket 断线重连优化

#### 2.2.1 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                     WebSocketClient                          │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ ReconnectMgr│  │ HeartbeatMgr│  │   MessageQueue      │  │
│  │  - strategy │  │  - interval │  │   - pendingMsgs     │  │
│  │  - backoff  │  │  - timeout  │  │   - maxsize         │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    QWebSocket                        │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

#### 2.2.2 重连配置

```cpp
struct ReconnectConfig {
    bool autoReconnect = true;        // 是否自动重连
    int maxReconnectAttempts = 10;    // 最大重连次数（0=无限）
    int baseDelayMs = 1000;           // 基础延迟
    int maxDelayMs = 30000;           // 最大延迟（30秒）
    double backoffMultiplier = 1.5;   // 退避乘数
    int connectionTimeoutMs = 10000;  // 连接超时
};

struct HeartbeatConfig {
    bool enabled = true;              // 是否启用心跳
    int intervalMs = 30000;           // 心跳间隔（30秒）
    int timeoutMs = 10000;            // 心跳超时（10秒）
    int maxMissedHeartbeats = 3;      // 最大丢失心跳数
};
```

#### 2.2.3 心跳机制

```
客户端                                    服务器
   │                                        │
   │────────── ping ──────────────────────>│
   │                                        │
   │<───────── pong ───────────────────────│
   │                                        │
   │        (30秒后)                        │
   │                                        │
   │────────── ping ──────────────────────>│
   │                                        │
   │   (10秒超时无响应)                      │
   │                                        │
   │──── 重连流程 ───────────────────────>│
```

#### 2.2.4 消息队列机制

```cpp
struct QueuedMessage {
    QString id;
    QJsonObject message;
    qint64 timestamp;
    int retryCount = 0;
    bool requiresAck = true;
};

class MessageQueue {
public:
    void enqueue(const QJsonObject& message);
    QJsonObject dequeue();
    void ackReceived(const QString& messageId);
    void clear();
    int size() const;
    void setMaxSize(int size);
    
private:
    QList<QueuedMessage> m_queue;
    int m_maxSize = 100;
};
```

#### 2.2.5 重连状态机

```
                    ┌──────────────┐
                    │   Disconnected │
                    └──────┬───────┘
                           │ connectToServer()
                           ▼
                    ┌──────────────┐
          ┌────────│  Connecting   │────────┐
          │        └──────────────┘        │
          │              │                 │
          │   connected  │    timeout/     │
          │              │    error        │
          │              ▼                 ▼
          │       ┌──────────────┐  ┌──────────────┐
          │       │  Connected   │  │ Reconnecting │
          │       └──────────────┘  └──────────────┘
          │              │                 │
          │              │ disconnected    │
          │              ▼                 │
          │       ┌──────────────┐         │
          └──────>│  Reconnecting│<────────┘
                  └──────────────┘
                         │
                         │ max attempts reached
                         ▼
                  ┌──────────────┐
                  │    Failed    │
                  └──────────────┘
```

#### 2.2.6 核心类设计

```cpp
// WebSocketClient 改进
class WebSocketClient : public QObject {
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    
    void setReconnectConfig(const ReconnectConfig& config);
    void setHeartbeatConfig(const HeartbeatConfig& config);
    
    void connectToServer(const QString& token);
    void disconnectFromServer();
    bool isConnected() const;
    ConnectionState connectionState() const;
    
    void sendMessage(int recipientId, const QString& text);
    void sendFileMessage(int recipientId, const QString& fileUrl, 
                         const QString& fileName, const QString& messageType);
    
signals:
    void connected();
    void disconnected();
    void reconnecting(int attempt, int delay);
    void reconnected();
    void connectionFailed();
    void errorOccurred(const QString& error);
    
    // ... 其他信号
    
private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);
    void onHeartbeatTimer();
    void onHeartbeatTimeout();
    void onReconnectTimer();
    
private:
    void startHeartbeat();
    void stopHeartbeat();
    void sendPing();
    void handlePong();
    
    void scheduleReconnect();
    void attemptReconnect();
    void resetReconnectAttempts();
    
    void enqueueMessage(const QJsonObject& message);
    void flushMessageQueue();
    
    QWebSocket m_socket;
    QString m_token;
    
    // 重连相关
    ReconnectConfig m_reconnectConfig;
    int m_reconnectAttempts = 0;
    QTimer* m_reconnectTimer;
    QTimer* m_connectionTimer;
    
    // 心跳相关
    HeartbeatConfig m_heartbeatConfig;
    QTimer* m_heartbeatTimer;
    QTimer* m_heartbeatTimeoutTimer;
    int m_missedHeartbeats = 0;
    
    // 消息队列
    MessageQueue m_messageQueue;
    
    // 状态
    ConnectionState m_state = ConnectionState::Disconnected;
};

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Failed
};
```

---

## 三、实现计划

### 3.1 阶段一：ApiService 重试机制（预计2天）

| 任务 | 工时 | 优先级 |
|------|------|--------|
| 创建 RetryPolicy 类 | 2h | 高 |
| 创建 ApiRequest 结构体 | 1h | 高 |
| 实现指数退避算法 | 1h | 高 |
| 实现错误分类处理 | 2h | 高 |
| 实现请求超时控制 | 2h | 中 |
| 集成到现有 ApiService | 3h | 高 |
| 单元测试 | 3h | 中 |

### 3.2 阶段二：WebSocket 心跳机制（预计1天）

| 任务 | 工时 | 优先级 |
|------|------|--------|
| 实现心跳定时器 | 1h | 高 |
| 实现 ping/pong 处理 | 2h | 高 |
| 实现心跳超时检测 | 1h | 高 |
| 后端添加 pong 响应 | 1h | 高 |
| 测试验证 | 2h | 中 |

### 3.3 阶段三：WebSocket 重连机制（预计2天）

| 任务 | 工时 | 优先级 |
|------|------|--------|
| 实现重连状态机 | 2h | 高 |
| 实现指数退避重连 | 2h | 高 |
| 实现连接超时检测 | 1h | 高 |
| 添加重连信号通知 | 1h | 中 |
| UI 显示重连状态 | 2h | 中 |
| 测试验证 | 2h | 中 |

### 3.4 阶段四：消息队列机制（预计1天）

| 任务 | 工时 | 优先级 |
|------|------|--------|
| 创建 MessageQueue 类 | 2h | 高 |
| 实现消息入队/出队 | 1h | 高 |
| 实现消息确认机制 | 2h | 高 |
| 断线时消息缓存 | 1h | 高 |
| 重连后消息重发 | 2h | 高 |
| 测试验证 | 2h | 中 |

### 3.5 阶段五：集成测试与优化（预计1天）

| 任务 | 工时 | 优先级 |
|------|------|--------|
| 网络模拟测试 | 2h | 高 |
| 压力测试 | 2h | 中 |
| 性能优化 | 2h | 中 |
| 文档更新 | 1h | 低 |

---

## 四、测试方案

### 4.1 ApiService 测试用例

| 用例 | 描述 | 预期结果 |
|------|------|----------|
| 正常请求 | 发送正常请求 | 成功返回 |
| 网络断开 | 断开网络后请求 | 自动重试3次后失败 |
| 服务器500 | 服务器返回500 | 自动重试成功 |
| 认证失败 | 返回401 | 不重试，触发登录 |
| 请求超时 | 请求超过超时时间 | 取消请求，触发超时信号 |
| 并发请求 | 同时发送多个请求 | 按顺序处理，各自重试 |

### 4.2 WebSocket 测试用例

| 用例 | 描述 | 预期结果 |
|------|------|----------|
| 正常连接 | 连接服务器 | 成功连接 |
| 心跳正常 | 发送ping | 收到pong |
| 心跳超时 | 服务器不响应 | 触发重连 |
| 网络断开 | 断开网络 | 自动重连 |
| 重连成功 | 网络恢复 | 重连成功，发送队列消息 |
| 重连失败 | 服务器不可用 | 达到最大重试后失败 |
| 消息队列 | 断线时发送消息 | 重连后发送成功 |

---

## 五、风险评估

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| 重试风暴 | 服务器压力 | 中 | 限制并发重试数 |
| 内存泄漏 | 程序崩溃 | 低 | 定期清理超时请求 |
| 消息重复 | 业务错误 | 中 | 消息ID去重 |
| 重连死循环 | 资源耗尽 | 低 | 最大重试次数限制 |

---

## 六、性能指标

| 指标 | 目标值 |
|------|--------|
| 请求重试成功率 | > 95% |
| 重连成功率 | > 99% |
| 心跳响应时间 | < 100ms |
| 消息队列最大延迟 | < 5s |
| 内存占用增量 | < 10MB |

---

## 七、后续优化方向

1. **请求优先级队列**：重要请求优先处理
2. **离线消息同步**：断线期间消息服务器缓存
3. **多服务器支持**：支持备用服务器切换
4. **网络质量检测**：根据网络状况调整策略
5. **请求去重**：防止重复请求

---

*文档版本：v1.0*
*最后更新：2026年2月*
