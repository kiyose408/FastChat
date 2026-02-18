# FastChat - 跨平台即时通讯软件

## 项目简介

FastChat 是一款基于 Qt 框架开发的跨平台即时通讯应用，采用 C++/Qt 6 构建客户端，Node.js + PostgreSQL 构建服务端，支持实时消息收发、文件传输、好友管理等核心功能。

| 项目 | 说明 |
|------|------|
| 版本 | v0.11 |
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
| **用户界面** | 无边框窗口设计 | ✅ |
| | 自定义消息气泡 | ✅ |
| | 系统托盘支持 | ✅ |
| | 头像上传与裁切 | ✅ |

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
│   │   ├── ApiService             # HTTP API 服务
│   │   ├── WebSocketClient        # WebSocket 客户端
│   │   ├── SessionManager         # 会话管理
│   │   ├── FriendModel            # 好友数据模型
│   │   └── MessageModel           # 消息数据模型
│   ├── ui/                        # 用户界面
│   │   ├── ChatMainWindow         # 主聊天窗口
│   │   ├── LoginDialog            # 登录对话框
│   │   ├── RegisterDialog         # 注册对话框
│   │   ├── FriendManagementDialog # 好友管理
│   │   ├── SearchResultsDialog    # 搜索结果
│   │   └── ImageCropDialog        # 图片裁切
│   └── utils/                     # 工具类
├── resources/                     # 资源文件
├── docs/                          # 文档
└── scripts/                       # 构建脚本

fastchat-backend/                  # 后端项目
├── src/
│   ├── routes/                    # API 路由
│   │   ├── auth.js                # 认证接口
│   │   ├── friends.js             # 好友接口
│   │   ├── messages.js            # 消息接口
│   │   └── upload.js              # 上传接口
│   ├── models/                    # 数据模型
│   ├── middleware/                # 中间件
│   └── config/                    # 配置
├── uploads/                       # 上传文件
└── init.sql                       # 数据库初始化
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
psql -U postgres -d fastchat -f init.sql

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

## 使用说明

详细使用说明请参阅 [软件说明书](docs/MANUAL.md)。

### 基本操作

1. **注册账号**：首次使用需要注册账号
2. **添加好友**：点击左下角 "+" 按钮搜索并添加好友
3. **开始聊天**：点击好友列表中的好友开始聊天
4. **发送文件**：点击聊天区域的图片/文件按钮发送
5. **更换头像**：点击左上角头像区域上传新头像

### 系统托盘

- 关闭窗口时程序最小化到系统托盘
- 点击托盘图标恢复窗口
- 右键托盘图标显示菜单

## 开发文档

- [软件说明书](docs/MANUAL.md) - 详细功能说明和操作指南
- [开发日志](docs/CHANGELOG.md) - 版本更新记录
- [开发计划](docs/DEVELOPMENT_PLAN.md) - 未来开发规划

## 许可证

本项目采用 MIT 许可证。

---

*FastChat v0.11 - 2026年2月*
