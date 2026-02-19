# FastChat - 跨平台即时通讯软件

## 项目简介

FastChat 是一款基于 Qt 框架开发的跨平台即时通讯应用，采用 C++/Qt 6 构建客户端，Node.js + PostgreSQL 构建服务端，支持实时消息收发、文件传输、好友管理等核心功能。

| 项目 | 说明 |
|------|------|
| 版本 | v0.12 |
| 状态 | Beta |
| 许可证 | MIT |
| 平台 | Windows 10/11 (可扩展至 macOS/Linux) |

## 功能特性

### 核心功能

| 模块 | 功能 | 状态 |
|------|------|------|
| **用户认证** | 用户注册/登录 | ✅ |
| | JWT Token 认证 | ✅ |
| | 会话管理 | ✅ |
| **好友管理** | 搜索用户 | ✅ |
| | 发送/接受/拒绝好友请求 | ✅ |
| | 删除好友 | ✅ |
| | 修改好友备注 | ✅ |
| | 好友在线状态显示 | ✅ |
| | 好友请求红点通知 | ✅ |
| **消息通讯** | 文本消息收发 | ✅ |
| | 图片发送与预览 | ✅ |
| | 文件发送与下载 | ✅ |
| | 消息历史记录 | ✅ |
| | 消息已读状态 | ✅ |
| | 消息撤回 | ✅ |
| | 消息搜索 | ✅ |
| **实时通讯** | WebSocket 长连接 | ✅ |
| | 实时消息推送 | ✅ |
| | 心跳保活 | ✅ |
| | 自动重连 | ✅ |
| | 断线消息队列 | ✅ |
| **用户界面** | 无边框窗口设计 | ✅ |
| | 自定义消息气泡 | ✅ |
| | 系统托盘支持 | ✅ |
| | 头像上传与裁切 | ✅ |
| **网络可靠性** | HTTP 请求重试机制 | ✅ |
| | 指数退避策略 | ✅ |

## 技术栈

### 前端 (客户端)

| 技术 | 版本 | 用途 |
|------|------|------|
| Qt | 6.9.0 | GUI 框架 |
| C++ | C++17 | 核心语言 |
| CMake | 3.16+ | 构建系统 |
| Qt Network | - | HTTP/WebSocket 通信 |

### 后端 (服务端)

| 技术 | 版本 | 用途 |
|------|------|------|
| Node.js | 18+ | 运行环境 |
| Express | 4.x | Web 框架 |
| PostgreSQL | 14+ | 数据库 |
| ws | - | WebSocket 服务器 |
| JWT | - | 身份认证 |
| bcryptjs | - | 密码加密 |
| multer | - | 文件上传 |

## 项目结构

```
fastchat/                          # 前端项目
├── src/
│   ├── core/                      # 核心业务逻辑
│   │   ├── AppConfig.h            # 应用配置常量
│   │   ├── ApiService             # HTTP API 服务
│   │   ├── WebSocketClient        # WebSocket 客户端
│   │   ├── SessionManager         # 会话管理
│   │   ├── ConfigManager          # 配置管理
│   │   ├── FriendModel            # 好友数据模型
│   │   ├── MessageModel           # 消息数据模型
│   │   ├── MessageQueue           # 消息队列
│   │   ├── RetryPolicy            # 重试策略
│   │   └── ImageLoader            # 图片加载工具
│   ├── ui/                        # 用户界面
│   │   ├── ChatMainWindow         # 主聊天窗口
│   │   ├── LoginDialog            # 登录对话框
│   │   ├── RegisterDialog         # 注册对话框
│   │   ├── FriendManagementDialog # 好友管理
│   │   ├── FriendInfoDialog       # 好友信息
│   │   ├── FriendDelegate         # 好友列表委托
│   │   ├── MessageDelegate        # 消息列表委托
│   │   ├── SearchResultsDialog    # 搜索结果
│   │   ├── ImageCropDialog        # 图片裁切
│   │   ├── AvatarHelper           # 头像绘制工具
│   │   └── FramelessWindowHelper  # 无边框窗口辅助
│   ├── utils/                     # 工具类
│   │   └── logger                 # 日志工具
│   └── main.cpp                   # 程序入口
├── resources/                     # 资源文件
├── docs/                          # 文档
│   ├── MANUAL.md                  # 软件说明书
│   ├── ARCHITECTURE.md            # 架构说明
│   ├── CHANGELOG.md               # 开发日志
│   └── DEVELOPMENT_PLAN.md        # 开发计划
└── scripts/                       # 构建脚本

fastchat-backend/                  # 后端项目
├── src/
│   ├── config/                    # 配置
│   │   └── db.js                  # 数据库配置
│   ├── middleware/                # 中间件
│   │   └── auth.js                # JWT 认证
│   ├── models/                    # 数据模型
│   │   ├── User.js
│   │   ├── Friend.js
│   │   └── Message.js
│   ├── routes/                    # API 路由
│   │   ├── auth.js
│   │   ├── friends.js
│   │   ├── messages.js
│   │   └── upload.js
│   ├── websocket/                 # WebSocket 模块
│   │   ├── index.js               # 服务器初始化
│   │   └── handlers.js            # 消息处理器
│   ├── utils/
│   │   └── helpers.js
│   └── server.js                  # 服务入口
├── migrations/                    # 数据库迁移
├── scripts/                       # 工具脚本
├── tests/                         # 测试文件
└── uploads/                       # 上传文件
```

## 快速开始

### 环境要求

- Qt 6.9.0+ (MinGW 64-bit)
- CMake 3.16+
- Node.js 18+
- PostgreSQL 14+

### 后端部署

```bash
# 1. 进入后端目录
cd fastchat-backend

# 2. 安装依赖
npm install

# 3. 创建数据库
psql -U postgres -c "CREATE DATABASE fastchat;"

# 4. 初始化数据表
psql -U postgres -d fastchat -f migrations/001-init.sql

# 5. 配置环境变量
cp .env.example .env
# 编辑 .env 文件，配置数据库连接和 JWT 密钥

# 6. 启动服务
npm start
```

### 前端编译

```powershell
# 1. 进入前端目录
cd fastchat

# 2. 编译项目
powershell -ExecutionPolicy Bypass -File scripts/compile.ps1

# 3. 运行程序
./build/Release/fastchat.exe
```

### 环境变量配置

```env
# .env 文件示例
PORT=3000
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASSWORD=your_password
DB_NAME=fastchat
JWT_SECRET=your_jwt_secret_key
```

## API 接口

### 认证接口

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | /api/auth/register | 用户注册 |
| POST | /api/auth/login | 用户登录 |

### 好友接口

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | /api/friends | 获取好友列表 |
| GET | /api/friends/requests | 获取好友请求 |
| GET | /api/friends/search?q=xxx | 搜索用户 |
| POST | /api/friends/request | 发送好友请求 |
| POST | /api/friends/accept | 接受好友请求 |
| POST | /api/friends/reject | 拒绝好友请求 |
| DELETE | /api/friends/:id | 删除好友 |
| PUT | /api/friends/:id/note | 修改好友备注 |

### 消息接口

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | /api/messages/send | 发送消息 |
| GET | /api/messages/conversation/:id | 获取对话记录 |
| POST | /api/messages/mark-read | 标记已读 |
| GET | /api/messages/search?q=xxx | 搜索消息 |

### 上传接口

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | /api/upload/avatar | 上传头像 |
| POST | /api/upload/image | 上传图片 |
| POST | /api/upload/file | 上传文件 |

## 文档

- [软件说明书](docs/MANUAL.md) - 详细功能说明和操作指南
- [架构说明](docs/ARCHITECTURE.md) - 系统架构设计
- [开发日志](docs/CHANGELOG.md) - 版本更新记录
- [开发计划](docs/DEVELOPMENT_PLAN.md) - 未来开发规划

## 许可证

本项目采用 MIT 许可证。

---

*FastChat v0.12 - 2026年2月*
