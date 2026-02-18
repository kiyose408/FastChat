# FastChat 开发日志

## 项目概述

| 项目名称 | FastChat |
|----------|----------|
| 项目类型 | 即时通讯应用 |
| 前端技术 | Qt 6.9.0 (C++/Qt Widgets) |
| 后端技术 | Node.js + Express + PostgreSQL |
| 开发周期 | 2026年2月 |
| 版本 | v0.1.0-beta |

---

## 开发进度记录

### 第一阶段：项目初始化

#### 2026-02-17 项目搭建

**功能实现：**
- 创建Qt CMake项目结构
- 设计主界面UI (ChatMainWindow)
- 设计登录界面UI (LoginDialog)
- 实现无边框窗口设计
- 添加自定义标题栏（最小化/最大化/关闭按钮）

**技术要点：**
```cpp
// 无边框窗口实现
setWindowFlags(Qt::FramelessWindowHint);

// 自定义标题栏拖拽
void mousePressEvent(QMouseEvent *event);
void mouseMoveEvent(QMouseEvent *event);
```

---

### 第二阶段：用户认证系统

#### 2026-02-17 登录/注册功能

**功能实现：**
- 实现登录对话框 (LoginDialog)
- 实现注册对话框 (RegisterDialog)
- 创建ApiService网络请求服务
- 实现SessionManager会话管理
- JWT Token存储与管理

**后端API对接：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/auth/register | POST | 用户注册 |
| /api/auth/login | POST | 用户登录 |

**Bug修复：**
- [已修复] 登录成功后Token未正确保存
- [已修复] 注册时邮箱验证格式问题

---

### 第三阶段：好友管理系统

#### 2026-02-17 好友功能开发

**功能实现：**
- 创建好友列表模型 (FriendModel)
- 实现好友列表委托 (FriendDelegate)
- 创建好友管理对话框 (FriendManagementDialog)
- 实现用户搜索功能
- 实现发送好友请求（带备注）
- 实现好友请求列表显示
- 实现接受/拒绝好友请求
- 创建好友信息对话框 (FriendInfoDialog)
- 实现删除好友功能（二次确认）
- 实现修改好友备注功能

**后端API对接：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/friends | GET | 获取好友列表 |
| /api/friends/request | POST | 发送好友请求 |
| /api/friends/requests | GET | 获取好友请求列表 |
| /api/friends/accept | POST | 接受好友请求 |
| /api/friends/reject | POST | 拒绝好友请求 |
| /api/friends/:friendId | DELETE | 删除好友 |
| /api/friends/:friendId/note | PUT | 修改好友备注 |
| /api/friends/search | GET | 搜索用户 |

**UI组件：**
- FriendRequestBadge: 好友请求红点徽章
- ClickableLabel: 可点击标签组件

---

### 第四阶段：消息系统

#### 2026-02-17 消息功能开发

**功能实现：**
- 创建消息模型 (MessageModel)
- 实现消息委托 (MessageDelegate)
- 实现消息气泡绘制（自定义样式）
- 实现发送文本消息
- 实现获取对话记录

**后端API对接：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/messages/send | POST | 发送消息 |
| /api/messages/conversation/:id | GET | 获取对话记录 |

---

### 第五阶段：实时通讯系统

#### 2026-02-17 WebSocket实现

**功能实现：**
- 创建WebSocketClient客户端
- 实现WebSocket连接认证
- 实现实时消息接收
- 实现实时好友请求通知
- 实现好友请求接受通知
- 实现被好友删除通知

**消息类型：**
| 类型 | 说明 |
|------|------|
| send_message | 发送文本消息 |
| send_file | 发送文件消息 |
| new_message | 收到新消息 |
| friend_request | 收到好友请求 |
| friend_request_accepted | 好友请求被接受 |
| friend_deleted | 被好友删除 |

**Bug修复：**
- [已修复] WebSocket重连导致重复连接
- [已修复] 消息发送后未显示在界面

---

### 第六阶段：文件传输系统

#### 2026-02-17 文件/图片发送

**功能实现：**
- 实现图片选择与上传
- 实现文件选择与上传
- 实现图片预览显示
- 实现文件下载功能
- 数据库添加文件类型字段

**后端API对接：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/upload/image | POST | 上传图片 |
| /api/upload/file | POST | 上传文件 |

**数据库迁移：**
```sql
ALTER TABLE messages ADD COLUMN message_type VARCHAR(20) DEFAULT 'text';
ALTER TABLE messages ADD COLUMN file_url TEXT;
ALTER TABLE messages ADD COLUMN file_name VARCHAR(255);
```

**Bug修复：**
- [已修复] 文件上传被类型过滤器拒绝
- [已修复] 图片预览比例变形

---

### 第七阶段：UI优化与Bug修复

#### 2026-02-17~18 消息气泡优化

**Bug记录：**

1. **英文消息意外换行问题**
   - 现象：发送包含英文的消息时，出现不受控制的换行
   - 原因：QTextDocument的HTML解析导致
   - 解决方案：改用QFontMetrics逐字符计算，使用drawText逐行绘制

2. **字符吞没问题**
   - 现象：输入"ass"只显示"as"
   - 原因：文本绘制区域宽度计算不准确
   - 解决方案：先分行计算实际宽度，再逐行绘制

**优化内容：**
- 图片预览改为等比例缩放
- 使用"Microsoft YaHei"字体
- 优化消息气泡绘制性能

---

### 第八阶段：打包发布

#### 2026-02-17 发布准备

**功能实现：**
- 创建打包脚本 (build-release.ps1)
- 使用windeployqt收集依赖
- 创建README说明文件
- 生成v0.1.0-beta版本

**发布包内容：**
```
FastChat-0.1.0-beta/
├── fastchat.exe
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── Qt6Network.dll
├── Qt6WebSockets.dll
├── platforms/
├── imageformats/
├── translations/
└── README.txt
```

---

## 已知问题列表

| 编号 | 问题描述 | 状态 | 优先级 |
|------|----------|------|--------|
| 1 | 用户在线状态未实现 | 待开发 | 高 |
| 2 | 消息已读状态未实现 | 待开发 | 高 |
| 3 | 消息撤回功能未实现 | 待开发 | 中 |
| 4 | 群组聊天功能未实现 | 待开发 | 中 |
| 5 | 用户头像功能未实现 | 待开发 | 中 |
| 6 | 系统托盘通知未实现 | 待开发 | 低 |
| 7 | 消息搜索功能未实现 | 待开发 | 低 |

---

## 技术债务

1. **测试覆盖**：缺少单元测试和集成测试
2. **错误处理**：前端错误提示不够友好
3. **性能优化**：大文件上传无进度显示
4. **代码注释**：部分代码缺少注释说明

---

## 版本历史

| 版本 | 日期 | 主要变更 |
|------|------|----------|
| v0.1.0-beta | 2026-02-17 | 首个测试版本发布 |

---

## 开发者备注

1. 后端服务默认地址：http://localhost:3000
2. WebSocket路径：ws://localhost:3000/websocket
3. 文件上传限制：10MB
4. 支持的图片格式：png, jpg, jpeg, gif, bmp, webp
