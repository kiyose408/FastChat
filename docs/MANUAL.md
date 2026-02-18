# FastChat 软件说明书

## 文档信息

| 项目 | 内容 |
|------|------|
| 软件名称 | FastChat |
| 版本号 | v0.11 |
| 编写日期 | 2026年2月 |
| 适用平台 | Windows 10/11 |

---

## 目录

1. [软件概述](#1-软件概述)
2. [系统要求](#2-系统要求)
3. [安装说明](#3-安装说明)
4. [功能说明](#4-功能说明)
5. [操作指南](#5-操作指南)
6. [技术架构](#6-技术架构)
7. [API接口文档](#7-api接口文档)
8. [数据库设计](#8-数据库设计)
9. [常见问题](#9-常见问题)
10. [维护指南](#10-维护指南)

---

## 1. 软件概述

### 1.1 简介

FastChat 是一款基于 Qt 框架开发的即时通讯应用，支持用户注册登录、好友管理、实时消息收发、文件传输、头像管理、消息搜索等功能。

### 1.2 功能特性

| 功能模块 | 功能描述 |
|----------|----------|
| 用户认证 | 用户注册、登录、JWT认证、会话管理 |
| 好友管理 | 搜索用户、添加好友、删除好友、修改备注、在线状态显示 |
| 实时通讯 | WebSocket实时消息收发、好友请求通知、已读回执、消息撤回 |
| 文件传输 | 图片发送、文件发送、在线预览、下载 |
| 头像管理 | 头像上传、图片裁切、头像显示 |
| 消息搜索 | 搜索历史消息、定位到聊天位置 |
| 系统托盘 | 后台运行、新消息通知、托盘菜单 |

### 1.3 技术栈

| 层级 | 技术 |
|------|------|
| 前端 | Qt 6.9.0 (C++/Qt Widgets) |
| 后端 | Node.js + Express |
| 数据库 | PostgreSQL |
| 通讯协议 | HTTP REST API + WebSocket |

---

## 2. 系统要求

### 2.1 客户端要求

| 项目 | 最低配置 | 推荐配置 |
|------|----------|----------|
| 操作系统 | Windows 10 | Windows 11 |
| 处理器 | Intel Core i3 | Intel Core i5及以上 |
| 内存 | 4GB | 8GB及以上 |
| 磁盘空间 | 200MB | 500MB |
| 网络 | 宽带网络 | 宽带网络 |

### 2.2 服务端要求

| 项目 | 要求 |
|------|------|
| Node.js | v18.0.0及以上 |
| PostgreSQL | v14.0及以上 |
| 内存 | 2GB及以上 |
| 端口 | 3000 (HTTP + WebSocket) |

---

## 3. 安装说明

### 3.1 客户端安装

1. 解压 `FastChat-0.11.zip` 到任意目录
2. 双击 `fastchat.exe` 启动程序
3. 首次使用需要注册账号

### 3.2 服务端部署

```bash
# 1. 进入后端目录
cd fastchat-backend

# 2. 安装依赖
npm install

# 3. 配置环境变量
# 创建 .env 文件，配置数据库连接信息

# 4. 初始化数据库
psql -U postgres -f init.sql

# 5. 启动服务
npm start
```

### 3.3 环境变量配置

```env
# .env 文件示例
PORT=3000
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASSWORD=your_password
DB_NAME=fastchat
JWT_SECRET=your_jwt_secret
```

---

## 4. 功能说明

### 4.1 用户认证模块

#### 4.1.1 用户注册
- 输入用户名、邮箱、密码
- 密码使用bcrypt加密存储
- 注册成功后自动跳转登录

#### 4.1.2 用户登录
- 输入用户名、密码
- 登录成功获取JWT Token
- Token存储在本地配置文件

### 4.2 好友管理模块

#### 4.2.1 搜索用户
- 点击左下角"+"按钮打开好友管理
- 输入用户名搜索
- 显示搜索结果列表

#### 4.2.2 添加好友
- 点击搜索结果中的"添加"按钮
- 可选填写好友请求备注
- 等待对方接受

#### 4.2.3 处理好友请求
- 收到请求时显示红点徽章
- 点击查看请求列表
- 选择接受或拒绝

#### 4.2.4 删除好友
- 点击好友头像右侧的"..."按钮
- 选择"删除好友"
- 确认后删除好友关系

#### 4.2.5 修改好友备注
- 点击好友头像右侧的"..."按钮
- 选择"修改备注"
- 输入新备注并保存

#### 4.2.6 查看好友信息
- 点击好友头像右侧的"..."按钮
- 选择"查看资料"
- 显示好友详细信息

### 4.3 消息模块

#### 4.3.1 发送文本消息
- 选择好友
- 在输入框输入消息
- 点击发送按钮或按Enter键

#### 4.3.2 发送图片
- 点击图片按钮
- 选择图片文件
- 自动上传并发送

#### 4.3.3 发送文件
- 点击文件按钮
- 选择任意文件
- 自动上传并发送

#### 4.3.4 查看图片/下载文件
- 点击图片消息在浏览器中打开
- 点击文件消息自动下载

#### 4.3.5 消息撤回
- 右键点击自己发送的消息
- 选择"撤回消息"
- 消息将被撤回

#### 4.3.6 消息搜索
- 在搜索框输入关键词
- 点击搜索按钮
- 在结果列表中点击定位到聊天

### 4.4 头像管理模块

#### 4.4.1 更换头像
- 点击左上角用户头像
- 选择图片文件
- 裁切图片（1:1比例）
- 确认后上传

### 4.5 系统托盘模块

#### 4.5.1 窗口管理
- 关闭窗口时最小化到托盘
- 点击托盘图标恢复窗口
- 双击托盘图标恢复窗口

#### 4.5.2 托盘通知
- 窗口不在前台时收到新消息通知
- 显示发送者和消息内容预览

#### 4.5.3 托盘菜单
- 右键托盘图标显示菜单
- 显示主窗口
- 退出程序

---

## 5. 操作指南

### 5.1 界面说明

```
┌─────────────────────────────────────────────────────────────────┐
│  ─ □ ✕  │                    FastChat                │ 用户头像 │
├──────────────┬──────────────────────────────────────────────────┤
│              │                                                  │
│   [搜索框]    │                                                  │
│              │              聊天区域                             │
│   [+] 好友   │                                                  │
│              │     ┌────────────────────────┐                   │
│   好友1      │     │     消息气泡           │                   │
│   好友2      │     └────────────────────────┘                   │
│   好友3      │                                                  │
│              │                                                  │
│              │  ┌────────────────────────────────────────────┐  │
│              │  │ [图片] [文件]  输入框...          [发送]   │  │
│              │  └────────────────────────────────────────────┘  │
└──────────────┴──────────────────────────────────────────────────┘
```

### 5.2 快捷操作

| 操作 | 说明 |
|------|------|
| Enter | 发送消息 |
| 点击好友 | 打开聊天 |
| 点击红点徽章 | 查看好友请求 |
| 点击图片消息 | 在浏览器中打开 |
| 点击文件消息 | 下载文件 |
| 右键消息 | 显示上下文菜单 |
| 点击头像 | 更换头像 |

### 5.3 状态说明

| 图标/状态 | 说明 |
|-----------|------|
| 绿色圆点 | 好友在线 |
| 灰色圆点 | 好友离线 |
| 红点徽章 | 有新的好友请求 |
| 蓝色双勾 | 消息已读 |
| 单勾 | 消息已发送 |

---

## 6. 技术架构

### 6.1 前端架构

```
fastchat/
├── src/
│   ├── main.cpp                 # 程序入口
│   ├── core/                    # 核心模块
│   │   ├── ApiService           # API请求服务
│   │   ├── WebSocketClient      # WebSocket客户端
│   │   ├── SessionManager       # 会话管理
│   │   ├── ConfigManager        # 配置管理
│   │   ├── FriendModel          # 好友数据模型
│   │   └── MessageModel         # 消息数据模型
│   ├── ui/                      # UI模块
│   │   ├── ChatMainWindow       # 主窗口
│   │   ├── LoginDialog          # 登录对话框
│   │   ├── RegisterDialog       # 注册对话框
│   │   ├── FriendManagementDialog # 好友管理
│   │   ├── FriendInfoDialog     # 好友信息
│   │   ├── SearchResultsDialog  # 搜索结果
│   │   ├── ImageCropDialog      # 图片裁切
│   │   ├── FriendDelegate       # 好友列表委托
│   │   ├── MessageDelegate      # 消息列表委托
│   │   └── ClickableLabel       # 可点击标签
│   └── utils/
│       └── logger               # 日志工具
├── resources/                   # 资源文件
└── scripts/                     # 构建脚本
```

### 6.2 后端架构

```
fastchat-backend/
├── src/
│   ├── server.js               # 服务入口
│   ├── config/
│   │   ├── db.js              # 数据库配置
│   │   └── websocket.js       # WebSocket配置
│   ├── routes/
│   │   ├── auth.js            # 认证路由
│   │   ├── friends.js         # 好友路由
│   │   ├── messages.js        # 消息路由
│   │   └── upload.js          # 上传路由
│   ├── models/
│   │   ├── User.js            # 用户模型
│   │   ├── Friend.js          # 好友模型
│   │   └── Message.js         # 消息模型
│   └── middleware/
│       └── auth.js            # 认证中间件
├── uploads/                    # 上传文件目录
└── init.sql                   # 数据库初始化
```

### 6.3 数据流

```
用户操作 → UI层 → ApiService/WebSocketClient → 后端API/WebSocket → 数据库
                ↓
           Model更新 → View刷新
```

---

## 7. API接口文档

### 7.1 认证接口

#### POST /api/auth/register
注册新用户

**请求体：**
```json
{
  "username": "string",
  "email": "string",
  "password": "string"
}
```

**响应：**
```json
{
  "message": "User registered successfully.",
  "user": { "id": 1, "username": "...", "email": "..." },
  "token": "jwt_token"
}
```

#### POST /api/auth/login
用户登录

**请求体：**
```json
{
  "username": "string",
  "password": "string"
}
```

### 7.2 好友接口

#### GET /api/friends
获取好友列表

**请求头：**
```
Authorization: Bearer <token>
```

**响应：**
```json
[
  { 
    "friend_id": 2, 
    "username": "userA", 
    "email": "a@email.com",
    "avatar_url": "/uploads/avatars/xxx.jpg",
    "is_online": true
  }
]
```

#### POST /api/friends/request
发送好友请求

**请求体：**
```json
{
  "friendId": 2,
  "note": "你好，我是xxx"
}
```

#### GET /api/friends/search?q=username
搜索用户

### 7.3 消息接口

#### POST /api/messages/send
发送消息

**请求体：**
```json
{
  "recipientId": 2,
  "content": "Hello"
}
```

#### GET /api/messages/conversation/:recipientId
获取对话记录

#### POST /api/messages/mark-read
标记消息已读

**请求体：**
```json
{
  "senderId": 2
}
```

#### GET /api/messages/search?q=keyword
搜索消息

### 7.4 上传接口

#### POST /api/upload/avatar
上传头像

**请求体：** multipart/form-data, 字段名: avatar

#### POST /api/upload/image
上传图片

**请求体：** multipart/form-data, 字段名: image

#### POST /api/upload/file
上传文件

**请求体：** multipart/form-data, 字段名: file

---

## 8. 数据库设计

### 8.1 用户表 (users)

| 字段 | 类型 | 说明 |
|------|------|------|
| id | SERIAL | 主键 |
| username | VARCHAR(50) | 用户名，唯一 |
| email | VARCHAR(100) | 邮箱，唯一 |
| password_hash | TEXT | 密码哈希 |
| avatar_url | TEXT | 头像URL |
| is_online | BOOLEAN | 在线状态 |
| last_seen | TIMESTAMP | 最后在线时间 |
| created_at | TIMESTAMP | 创建时间 |

### 8.2 好友表 (friends)

| 字段 | 类型 | 说明 |
|------|------|------|
| id | SERIAL | 主键 |
| user_id | INTEGER | 用户ID |
| friend_id | INTEGER | 好友ID |
| status | VARCHAR(20) | 状态：pending/accepted |
| note | TEXT | 好友备注 |
| created_at | TIMESTAMP | 创建时间 |

### 8.3 消息表 (messages)

| 字段 | 类型 | 说明 |
|------|------|------|
| id | SERIAL | 主键 |
| sender_id | INTEGER | 发送者ID |
| recipient_id | INTEGER | 接收者ID |
| content | TEXT | 消息内容 |
| message_type | VARCHAR(20) | 消息类型：text/image/file |
| file_url | TEXT | 文件URL |
| file_name | VARCHAR(255) | 文件名 |
| is_read | BOOLEAN | 已读状态 |
| read_at | TIMESTAMP | 已读时间 |
| is_recalled | BOOLEAN | 是否撤回 |
| recalled_at | TIMESTAMP | 撤回时间 |
| created_at | TIMESTAMP | 创建时间 |

---

## 9. 常见问题

### 9.1 无法连接服务器

**原因：** 后端服务未启动或地址配置错误

**解决方案：**
1. 确认后端服务已启动
2. 检查服务地址是否正确（默认 localhost:3000）
3. 检查防火墙设置

### 9.2 登录失败

**原因：** 用户名或密码错误

**解决方案：**
1. 检查用户名和密码
2. 确认账号已注册
3. 检查数据库连接

### 9.3 消息发送失败

**原因：** WebSocket连接断开

**解决方案：**
1. 检查网络连接
2. 重新登录
3. 检查后端日志

### 9.4 文件上传失败

**原因：** 文件超过大小限制

**解决方案：**
1. 文件大小限制为10MB（头像5MB）
2. 检查文件格式是否支持

### 9.5 头像上传失败

**原因：** 图片格式不支持或文件过大

**解决方案：**
1. 支持格式：png, jpg, jpeg, gif, bmp, webp
2. 头像大小限制为5MB

### 9.6 搜索无结果

**原因：** 关键词不匹配或消息不存在

**解决方案：**
1. 尝试其他关键词
2. 确认消息存在

---

## 10. 维护指南

### 10.1 日志查看

**前端日志：**
- 在Qt Creator中查看控制台输出
- 使用qDebug()输出调试信息

**后端日志：**
- 查看终端输出
- 日志文件位置：待配置

### 10.2 数据库维护

```sql
-- 查看用户数量
SELECT COUNT(*) FROM users;

-- 查看消息数量
SELECT COUNT(*) FROM messages;

-- 查看在线用户
SELECT username FROM users WHERE is_online = true;

-- 清理测试数据
DELETE FROM messages WHERE sender_id = 1;
```

### 10.3 服务重启

```bash
# 停止服务
Ctrl + C

# 启动服务
npm start
```

### 10.4 版本更新

1. 备份数据库
2. 拉取最新代码
3. 运行数据库迁移脚本
4. 重新编译前端
5. 重启后端服务

---

## 附录

### A. 文件结构说明

| 目录 | 说明 |
|------|------|
| /src/core/ | 核心业务逻辑 |
| /src/ui/ | 用户界面组件 |
| /src/utils/ | 工具类 |
| /resources/ | 图标等资源文件 |
| /docs/ | 文档目录 |
| /scripts/ | 构建脚本 |

### B. WebSocket 消息类型

| 类型 | 方向 | 说明 |
|------|------|------|
| send_message | 客户端→服务器 | 发送消息 |
| send_file | 客户端→服务器 | 发送文件 |
| mark_read | 客户端→服务器 | 标记已读 |
| recall_message | 客户端→服务器 | 撤回消息 |
| new_message | 服务器→客户端 | 新消息通知 |
| message_sent | 服务器→客户端 | 消息发送成功 |
| message_recalled | 服务器→客户端 | 消息撤回通知 |
| messages_read | 服务器→客户端 | 已读回执 |
| friend_request | 服务器→客户端 | 好友请求通知 |
| user_status | 服务器→客户端 | 用户状态变化 |

### C. 联系方式

如有问题，请联系开发团队。

---

*文档版本：v1.1*
*最后更新：2026年2月*
