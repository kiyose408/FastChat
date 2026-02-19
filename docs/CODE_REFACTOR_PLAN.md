# FastChat 代码结构优化方案

## 一、当前问题总结

### 1.1 高优先级问题

| 问题 | 位置 | 影响 |
|------|------|------|
| ConfigManager 类型错误 | configmanager.h:12 | API设计错误 |
| 服务器URL硬编码 | 多处 | 维护困难 |
| ApiService重试机制未使用 | ApiService.cpp | 功能浪费 |

### 1.2 中优先级问题

| 问题 | 位置 | 影响 |
|------|------|------|
| 头像绘制逻辑重复 | 3个Delegate | 代码冗余 |
| 窗口拖动逻辑重复 | 2个窗口类 | 代码冗余 |
| 图片加载逻辑重复 | FriendDelegate/MessageDelegate | 代码冗余 |
| IAuthService未使用 | core/ | 死代码 |
| 冗余头文件包含 | SessionManager.cpp | 编译效率 |
| 冗余前向声明 | chatmainwindow.h | 代码混乱 |

### 1.3 低优先级问题

| 问题 | 位置 | 影响 |
|------|------|------|
| 文件命名不一致 | configmanager.h | 风格不统一 |
| 日志使用不一致 | 多处 | 风格不统一 |
| 颜色常量分散 | 各Delegate | 维护困难 |

---

## 二、优化流程

### 阶段一：基础修复（预计1小时）

```
步骤1: 修复 ConfigManager 类型错误
步骤2: 统一服务器地址配置
步骤3: 移除冗余头文件和前向声明
```

### 阶段二：代码整合（预计2小时）

```
步骤1: 创建 AvatarHelper 工具类（头像绘制）
步骤2: 创建 ImageLoader 工具类（图片加载）
步骤3: 创建 FramelessWindowHelper 工具类（窗口拖动）
```

### 阶段三：清理优化（预计1小时）

```
步骤1: 移除或完善 IAuthService/MockAuthService
步骤2: 统一日志使用方式
步骤3: 统一文件命名风格
```

---

## 三、具体优化方案

### 3.1 修复 ConfigManager 类型错误

**当前代码：**
```cpp
// configmanager.h:12
void setServerAddress(const QSize& addr);  // 错误类型
```

**修改为：**
```cpp
void setServerAddress(const QString& addr);
QString serverAddress() const;
```

### 3.2 统一服务器地址配置

**创建配置常量：**
```cpp
// core/AppConfig.h
#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

class AppConfig {
public:
    static QString serverHost() { return "localhost"; }
    static int serverPort() { return 3000; }
    static QString serverBaseUrl() { return "http://localhost:3000"; }
    static QString wsBaseUrl() { return "ws://localhost:3000"; }
    
private:
    AppConfig() = delete;
};

#endif // APPCONFIG_H
```

**修改位置：**
- ApiService.cpp:111 → `return AppConfig::serverBaseUrl();`
- WebSocketClient.cpp:28 → `QUrl url(AppConfig::wsBaseUrl() + "/websocket?token=" + token);`
- FriendDelegate.cpp:32 → `QString fullUrl = AppConfig::serverBaseUrl() + url;`
- MessageDelegate.cpp:79 → `QString fullUrl = AppConfig::serverBaseUrl() + url;`

### 3.3 创建 AvatarHelper 工具类

**新文件：ui/AvatarHelper.h**
```cpp
#ifndef AVATARHELPER_H
#define AVATARHELPER_H

#include <QPixmap>
#include <QString>
#include <QPainter>

class AvatarHelper {
public:
    static QPixmap drawCircularAvatar(const QPixmap& pixmap, int size);
    static QPixmap drawDefaultAvatar(const QString& name, int size, const QColor& bgColor);
    static void loadAvatarAsync(const QString& url, int size, std::function<void(QPixmap)> callback);
    
private:
    AvatarHelper() = delete;
};

#endif // AVATARHELPER_H
```

**使用位置：**
- FriendDelegate.cpp
- UserDelegate.cpp
- FriendRequestDelegate.cpp
- chatmainwindow.cpp（头像上传成功后）

### 3.4 创建 ImageLoader 工具类

**新文件：core/ImageLoader.h**
```cpp
#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>

class ImageLoader : public QObject {
    Q_OBJECT
    
public:
    static ImageLoader& instance();
    
    void load(const QString& url, int size, std::function<void(QPixmap)> callback);
    void clearCache();
    
private:
    ImageLoader();
    ~ImageLoader();
    
    QNetworkAccessManager m_manager;
    QMap<QString, QPixmap> m_cache;
};

#endif // IMAGELOADER_H
```

**使用位置：**
- FriendDelegate.cpp
- MessageDelegate.cpp

### 3.5 创建 FramelessWindowHelper 工具类

**新文件：ui/FramelessWindowHelper.h**
```cpp
#ifndef FRAMELESSWINDOWHELPER_H
#define FRAMELESSWINDOWHELPER_H

#include <QObject>
#include <QPoint>
#include <QMouseEvent>

class QWidget;

class FramelessWindowHelper : public QObject {
    Q_OBJECT
    
public:
    explicit FramelessWindowHelper(QWidget* parent);
    
    void handleMousePressEvent(QMouseEvent* event);
    void handleMouseMoveEvent(QMouseEvent* event);
    void handleMouseReleaseEvent(QMouseEvent* event);
    
private:
    QWidget* m_parent;
    bool m_dragging;
    QPoint m_dragPosition;
};

#endif // FRAMELESSWINDOWHELPER_H
```

**使用位置：**
- chatmainwindow.cpp
- FriendManagementDialog.cpp

### 3.6 移除冗余代码

**移除文件：**
- core/IAuthService.h（如果确认不再使用）
- core/MockAuthService.h
- core/MockAuthService.cpp

**或完善使用：**
```cpp
// 在 LoginDialog 中使用接口
class LoginDialog : public QDialog {
private:
    IAuthService* m_authService;  // 通过构造函数注入
};

// ApiService 实现 IAuthService
class ApiService : public IAuthService { ... };
```

### 3.7 修复冗余前向声明

**chatmainwindow.h 当前：**
```cpp
#include "core/FriendModel.h"
#include "core/MessageModel.h"
// ...
class FriendModel;    // 冗余
class MessageModel;   // 冗余
```

**修改为：**
```cpp
#include "core/FriendModel.h"
#include "core/MessageModel.h"
// 移除前向声明
```

### 3.8 统一日志使用

**创建宏定义：**
```cpp
// utils/Log.h
#ifndef LOG_H
#define LOG_H

#include <QDebug>

#define LOG_DEBUG(msg) qDebug() << msg
#define LOG_INFO(msg)  qInfo() << msg
#define LOG_WARN(msg)  qWarning() << msg
#define LOG_ERROR(msg) qCritical() << msg

#endif // LOG_H
```

---

## 四、优化后的目录结构

```
src/
├── core/                           # 核心业务逻辑
│   ├── AppConfig.h                 # [新增] 应用配置常量
│   ├── ApiService.h/cpp            # HTTP API服务
│   ├── ConfigManager.h/cpp         # 配置管理（修复类型）
│   ├── FriendModel.h/cpp           # 好友数据模型
│   ├── ImageLoader.h/cpp           # [新增] 图片加载工具
│   ├── MessageModel.h/cpp          # 消息数据模型
│   ├── MessageQueue.h/cpp          # 消息队列
│   ├── RetryPolicy.h/cpp           # 重试策略
│   ├── SessionManager.h/cpp        # 会话管理
│   └── WebSocketClient.h/cpp       # WebSocket客户端
│
├── ui/                             # 用户界面
│   ├── AvatarHelper.h/cpp          # [新增] 头像绘制工具
│   ├── chatmainwindow.h/cpp/ui     # 主窗口
│   ├── ClickableLabel.h/cpp        # 可点击标签
│   ├── FramelessWindowHelper.h/cpp # [新增] 无边框窗口辅助
│   ├── FriendDelegate.h/cpp        # 好友列表委托
│   ├── FriendInfoDialog.h/cpp/ui   # 好友信息对话框
│   ├── FriendManagementDialog.h/cpp/ui # 好友管理对话框
│   ├── FriendRequestDelegate.h/cpp # 好友请求委托
│   ├── ImageCropDialog.h/cpp       # 图片裁剪对话框
│   ├── logindialog.h/cpp/ui        # 登录对话框
│   ├── MessageDelegate.h/cpp       # 消息委托
│   ├── RegisterDialog.h/cpp/ui     # 注册对话框
│   ├── SearchResultsDialog.h/cpp   # 搜索结果对话框
│   └── UserDelegate.h/cpp          # 用户委托
│
├── utils/                          # 工具类
│   ├── Log.h                       # [新增] 日志宏
│   └── logger.h/cpp                # 日志工具
│
└── main.cpp                        # 程序入口
```

---

## 五、执行计划

| 阶段 | 任务 | 预计时间 | 优先级 |
|------|------|----------|--------|
| 1 | 创建 AppConfig.h | 10分钟 | 高 |
| 2 | 修复 ConfigManager 类型 | 10分钟 | 高 |
| 3 | 统一服务器地址引用 | 20分钟 | 高 |
| 4 | 创建 AvatarHelper | 30分钟 | 中 |
| 5 | 创建 ImageLoader | 30分钟 | 中 |
| 6 | 创建 FramelessWindowHelper | 20分钟 | 中 |
| 7 | 重构 Delegate 使用工具类 | 30分钟 | 中 |
| 8 | 移除冗余代码 | 15分钟 | 低 |
| 9 | 统一日志使用 | 15分钟 | 低 |
| 10 | 统一文件命名 | 10分钟 | 低 |

**总预计时间：约3小时**

---

## 六、风险与注意事项

1. **重构过程中保持编译通过**：每完成一个步骤后立即编译测试
2. **保留原有功能**：重构不应改变任何用户可见的功能
3. **逐步提交**：每个阶段完成后单独提交，便于回滚
4. **测试验证**：重构完成后进行全面功能测试

---

*文档版本：v1.0*
*创建日期：2026年2月*
