# FastChat 前端架构说明

## 文档信息

| 项目 | 内容 |
|------|------|
| 版本 | v1.0 |
| 日期 | 2026年2月 |
| 适用版本 | FastChat v0.12 |

---

## 一、架构概览

### 1.1 整体架构

FastChat 前端采用 **分层架构** 设计，将应用分为三层：

```
┌─────────────────────────────────────────────────────────────┐
│                      表现层 (UI Layer)                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Dialogs    │  │   Windows   │  │     Delegates       │  │
│  │  (对话框)   │  │   (窗口)    │  │     (委托绘制)      │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                      业务层 (Core Layer)                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ ApiService  │  │ WebSocket   │  │      Models         │  │
│  │ (HTTP服务)  │  │  Client     │  │   (数据模型)        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                      基础层 (Foundation Layer)               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ SessionMgr  │  │ ConfigMgr   │  │     Utils           │  │
│  │ (会话管理)  │  │ (配置管理)  │  │    (工具类)         │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 设计原则

| 原则 | 说明 |
|------|------|
| 单一职责 | 每个类只负责一个功能 |
| 开闭原则 | 对扩展开放，对修改关闭 |
| 依赖倒置 | 高层模块不依赖低层模块 |
| 接口隔离 | 使用小接口而非大接口 |

---

## 二、模块详解

### 2.1 核心层 (Core)

#### 2.1.1 AppConfig

**职责**：统一管理应用配置常量

```cpp
class AppConfig {
public:
    static QString serverHost();      // 服务器主机
    static int serverPort();          // 服务器端口
    static QString serverBaseUrl();   // HTTP 基础 URL
    static QString wsBaseUrl();       // WebSocket 基础 URL
};
```

**设计理由**：
- 避免硬编码服务器地址
- 便于环境切换（开发/测试/生产）

#### 2.1.2 ApiService

**职责**：HTTP API 请求服务

```cpp
class ApiService : public QObject {
public:
    // 用户认证
    void registerUser(...);
    void login(...);
    
    // 好友管理
    void searchUsers(...);
    void sendFriendRequest(...);
    void getFriends(...);
    
    // 消息管理
    void sendMessage(...);
    void getConversation(...);
    
    // 文件上传
    void uploadImage(...);
    void uploadFile(...);
    void uploadAvatar(...);

private:
    QNetworkAccessManager m_netManager;
    RetryPolicy* m_retryPolicy;
    QMap<QString, PendingRequest> m_pendingRequests;
};
```

**关键特性**：
- 请求重试机制（指数退避）
- 超时控制
- 错误分类处理

#### 2.1.3 WebSocketClient

**职责**：WebSocket 实时通信

```cpp
class WebSocketClient : public QObject {
public:
    void connectToServer(const QString& token);
    void disconnectFromServer();
    bool isConnected() const;
    
    void sendMessage(int recipientId, const QString& text);
    void sendFileMessage(...);
    void markMessagesAsRead(int senderId);
    void recallMessage(int messageId, int recipientId);

signals:
    void connected();
    void disconnected();
    void reconnecting(int attempt, int delay);
    void reconnected();
    void messageReceived(const QJsonObject& message);

private:
    QWebSocket m_socket;
    ConnectionState m_state;
    
    // 心跳机制
    HeartbeatConfig m_heartbeatConfig;
    QTimer* m_heartbeatTimer;
    
    // 重连机制
    ReconnectConfig m_reconnectConfig;
    int m_reconnectAttempts;
    
    // 消息队列
    MessageQueue m_messageQueue;
};
```

**关键特性**：
- 心跳检测（30秒间隔）
- 自动重连（指数退避）
- 断线消息队列

#### 2.1.4 RetryPolicy

**职责**：HTTP 请求重试策略

```cpp
struct RetryConfig {
    int maxRetries = 3;           // 最大重试次数
    int baseDelayMs = 1000;       // 基础延迟
    int maxDelayMs = 10000;       // 最大延迟
    double backoffMultiplier = 2.0; // 退避乘数
};

class RetryPolicy {
public:
    bool shouldRetry(QNetworkReply::NetworkError error, int statusCode);
    int calculateDelay(int retryCount);
    bool canRetry(int retryCount);
};
```

**重试策略**：
```
第1次重试: 1秒后
第2次重试: 2秒后
第3次重试: 4秒后
```

#### 2.1.5 MessageQueue

**职责**：断线时缓存待发送消息

```cpp
class MessageQueue {
public:
    QString enqueue(const QJsonObject& message);
    QueuedMessage dequeue();
    void clear();
    int size() const;

private:
    QList<QueuedMessage> m_queue;
    int m_maxSize = 100;
};
```

#### 2.1.6 数据模型

**FriendModel** - 好友列表模型

```cpp
struct FriendData {
    int friendId;
    QString username;
    QString email;
    QString avatarUrl;
    bool isOnline;
    QString note;
    int unreadCount;
};

class FriendModel : public QAbstractListModel {
public:
    enum Roles {
        FriendIdRole = Qt::UserRole + 1,
        UsernameRole,
        AvatarRole,
        IsOnlineRole,
        NoteRole,
        UnreadCountRole
    };
    
    void addFriend(const FriendData& friend);
    void updateOnlineStatus(int friendId, bool isOnline);
    void clear();
};
```

**MessageModel** - 消息列表模型

```cpp
struct MessageData {
    int messageId;
    bool isSelf;
    QString text;
    QString time;
    QString messageType;
    QString fileUrl;
    QString fileName;
    bool isRead;
    bool isRecalled;
};

class MessageModel : public QAbstractListModel {
public:
    enum Roles {
        MessageIdRole = Qt::UserRole + 1,
        IsSelfRole,
        TextRole,
        TimeRole,
        MessageTypeRole,
        FileUrlRole,
        IsReadRole,
        IsRecalledRole
    };
    
    void addMessage(const MessageData& msg);
    void markAsRead(int messageId);
    void recallMessage(int messageId);
};
```

### 2.2 表现层 (UI)

#### 2.2.1 主窗口

**ChatMainWindow** - 主聊天窗口

```
┌─────────────────────────────────────────────────────────────┐
│  ─ □ ✕  │                    FastChat                │ 头像 │
├──────────────┬──────────────────────────────────────────────┤
│              │                                              │
│   [搜索框]    │                                              │
│              │              聊天区域                         │
│   [+] 好友   │                                              │
│              │     ┌────────────────────────┐               │
│   好友列表    │     │     消息气泡           │               │
│              │     └────────────────────────┘               │
│              │                                              │
│              │  ┌────────────────────────────────────────┐  │
│              │  │ [图片] [文件]  输入框...        [发送]  │  │
│              │  └────────────────────────────────────────┘  │
└──────────────┴──────────────────────────────────────────────┘
```

**职责**：
- 整合所有功能模块
- 处理用户交互
- 管理子窗口

#### 2.2.2 委托 (Delegates)

**FriendDelegate** - 好友列表项绘制

```cpp
class FriendDelegate : public QStyledItemDelegate {
public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, 
                   const QModelIndex& index) const override;

private:
    void drawAvatar(QPainter* painter, const QRect& rect, const QString& url) const;
    void drawOnlineIndicator(QPainter* painter, const QRect& rect, bool isOnline) const;
    void drawUnreadBadge(QPainter* painter, const QRect& rect, int count) const;
};
```

**MessageDelegate** - 消息气泡绘制

```cpp
class MessageDelegate : public QStyledItemDelegate {
public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, 
                   const QModelIndex& index) const override;

private:
    void paintTextMessage(...);
    void paintImageMessage(...);
    void paintFileMessage(...);
    void paintRecalledMessage(...);
};
```

#### 2.2.3 工具类

**AvatarHelper** - 头像绘制工具

```cpp
class AvatarHelper {
public:
    static QPixmap drawCircularAvatar(const QPixmap& source, int size);
    static QPixmap drawDefaultAvatar(const QString& name, int size, const QColor& bgColor);
};
```

**FramelessWindowHelper** - 无边框窗口辅助

```cpp
class FramelessWindowHelper {
public:
    void handleMousePressEvent(QMouseEvent* event);
    void handleMouseMoveEvent(QMouseEvent* event);
    void handleMouseReleaseEvent(QMouseEvent* event);
};
```

**ImageLoader** - 图片加载工具

```cpp
class ImageLoader {
public:
    static ImageLoader& instance();
    void load(const QString& url, std::function<void(QPixmap)> callback);
    void clearCache();
    bool isCached(const QString& url) const;
};
```

### 2.3 基础层 (Foundation)

#### 2.3.1 SessionManager

**职责**：用户会话管理（单例）

```cpp
class SessionManager {
public:
    static SessionManager& instance();
    
    QString token() const;
    void setToken(const QString& token);
    
    int userId() const;
    QString username() const;
    void setUserInfo(int userId, const QString& username);
    
    bool isLoggedIn() const;
    void logout();
    
    void save();    // 持久化到本地
    void load();    // 从本地加载

private:
    QString m_token;
    int m_userId;
    QString m_username;
    QSettings m_settings;
};
```

#### 2.3.2 ConfigManager

**职责**：应用配置管理（单例）

```cpp
class ConfigManager {
public:
    static ConfigManager& instance();
    
    QString serverAddress() const;
    void setServerAddress(const QString& addr);
    
    QSize windowSize() const;
    void setWindowSize(const QSize& size);
    
    bool rememberMe() const;
    void setRememberMe(bool remember);

private:
    QSettings m_settings;
};
```

---

## 三、数据流

### 3.1 登录流程

```
用户输入 → LoginDialog → ApiService.login()
                              ↓
                        HTTP POST /api/auth/login
                              ↓
                        服务器返回 JWT Token
                              ↓
                        SessionManager.setToken()
                              ↓
                        WebSocketClient.connectToServer()
                              ↓
                        ChatMainWindow 显示
```

### 3.2 发送消息流程

```
用户输入 → ChatMainWindow → WebSocketClient.sendMessage()
                                    ↓
                              WebSocket send_message
                                    ↓
                              服务器存储并转发
                                    ↓
                              WebSocket message_sent
                                    ↓
                              MessageModel.addMessage()
                                    ↓
                              View 刷新
```

### 3.3 接收消息流程

```
服务器 → WebSocket new_message → WebSocketClient.messageReceived()
                                          ↓
                                    MessageModel.addMessage()
                                          ↓
                                    View 刷新
                                          ↓
                                    (如果窗口不在前台)
                                          ↓
                                    系统托盘通知
```

---

## 四、网络可靠性设计

### 4.1 HTTP 重试机制

```
请求失败 → 判断错误类型
              ↓
        ┌─────┴─────┐
        ↓           ↓
    可重试错误    不可重试错误
    (网络/5xx)    (401/403/404)
        ↓           ↓
    指数退避重试   直接返回错误
        ↓
    达到最大重试次数 → 返回错误
```

### 4.2 WebSocket 重连机制

```
连接断开 → 判断是否自动重连
              ↓
        ┌─────┴─────┐
        ↓           ↓
      是          否
        ↓           ↓
    指数退避等待   显示连接失败
        ↓
    尝试重连 → 成功 → 刷新消息队列
              ↓
            失败 → 继续等待重试
```

### 4.3 心跳机制

```
客户端                              服务器
   │                                  │
   │──── ping (每30秒) ─────────────→│
   │                                  │
   │←─── pong ────────────────────────│
   │                                  │
   │   (超时10秒无响应)                │
   │                                  │
   │──── 重连流程 ─────────────────→│
```

---

## 五、扩展性设计

### 5.1 添加新的 API 接口

1. 在 `ApiService` 中添加方法
2. 定义信号
3. 在调用处连接信号

### 5.2 添加新的消息类型

1. 在 `MessageModel` 中添加角色
2. 在 `MessageDelegate` 中添加绘制逻辑
3. 在 `WebSocketClient` 中添加处理

### 5.3 添加新的界面

1. 创建对话框/窗口类
2. 在 `ChatMainWindow` 中集成
3. 连接信号槽

---

## 六、性能优化

### 6.1 已实现的优化

| 优化项 | 说明 |
|--------|------|
| 图片缓存 | ImageLoader 单例缓存 |
| 头像缓存 | FriendDelegate 静态缓存 |
| 消息虚拟化 | QListView 自动虚拟化 |
| 异步加载 | 网络请求异步处理 |

### 6.2 待优化项

| 优化项 | 建议 |
|--------|------|
| 大图片压缩 | 上传前压缩图片 |
| 消息分页加载 | 滚动加载历史消息 |
| 本地数据库 | SQLite 缓存消息 |

---

*文档版本：v1.0*
*最后更新：2026年2月*
