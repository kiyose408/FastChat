# FastChat 下一阶段开发计划

## 开发周期

| 项目 | 内容 |
|------|------|
| 开发周期 | 3天 |
| 开始日期 | 待定 |
| 目标版本 | v0.2.0 |

---

## 任务概览

| 编号 | 功能 | 优先级 | 预计工时 | 分配日期 |
|------|------|--------|----------|----------|
| 1 | 用户在线状态 | 高 | 4小时 | Day 1 |
| 2 | 消息已读状态 | 高 | 4小时 | Day 1 |
| 3 | 消息撤回功能 | 中 | 3小时 | Day 2 |
| 4 | 群组聊天功能 | 中 | 6小时 | Day 2-3 |
| 5 | 用户头像功能 | 中 | 4小时 | Day 2 |
| 6 | 系统托盘通知 | 低 | 2小时 | Day 3 |
| 7 | 消息搜索功能 | 低 | 3小时 | Day 3 |

---

## Day 1：高优先级功能

### 任务1：用户在线状态（4小时）

#### 后端开发（2小时）

**数据库设计：**
```sql
-- 添加用户在线状态字段
ALTER TABLE users ADD COLUMN is_online BOOLEAN DEFAULT FALSE;
ALTER TABLE users ADD COLUMN last_seen TIMESTAMP;
```

**WebSocket改造：**
```javascript
// websocket.js
// 用户连接时设置在线状态
clients.set(userId, ws);
await pool.query('UPDATE users SET is_online = true WHERE id = $1', [userId]);

// 用户断开时设置离线状态
await pool.query('UPDATE users SET is_online = false, last_seen = NOW() WHERE id = $1', [userId]);

// 广播在线状态变化
function broadcastOnlineStatus(userId, isOnline) {
    // 通知所有好友
}
```

**新增API：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/users/:id/status | GET | 获取用户在线状态 |

#### 前端开发（2小时）

**FriendModel扩展：**
```cpp
struct FriendData {
    int id;
    QString nickname;
    bool isOnline;      // 新增
    QString lastSeen;   // 新增
    int unreadCount;
};
```

**FriendDelegate改造：**
- 在好友头像右下角显示在线状态圆点
- 绿色：在线
- 灰色：离线
- 显示最后在线时间

**WebSocket处理：**
```cpp
// 处理好友上线/下线通知
void onWebSocketUserOnline(int userId, bool isOnline);
```

---

### 任务2：消息已读状态（4小时）

#### 后端开发（2小时）

**数据库设计：**
```sql
-- 添加消息已读状态
ALTER TABLE messages ADD COLUMN is_read BOOLEAN DEFAULT FALSE;
ALTER TABLE messages ADD COLUMN read_at TIMESTAMP;

-- 创建未读消息统计视图
CREATE VIEW unread_count AS
SELECT recipient_id, sender_id, COUNT(*) as count
FROM messages
WHERE is_read = false
GROUP BY recipient_id, sender_id;
```

**新增API：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/messages/mark-read | POST | 标记消息已读 |
| /api/messages/unread-count | GET | 获取未读消息数 |

**WebSocket消息类型：**
```javascript
// 消息已读回执
{
    type: 'message_read',
    messageId: 123,
    readAt: '2026-02-18T10:00:00Z'
}
```

#### 前端开发（2小时）

**MessageModel扩展：**
```cpp
struct MessageData {
    bool isSelf;
    QString text;
    QString time;
    bool isRead;      // 新增
    QString readAt;   // 新增
};
```

**UI改造：**
- 已读消息显示"已读"状态
- 未读消息显示"未读"状态
- 好友列表显示未读消息数徽章

---

## Day 2：中优先级功能

### 任务3：消息撤回功能（3小时）

#### 后端开发（1.5小时）

**数据库设计：**
```sql
-- 添加消息撤回状态
ALTER TABLE messages ADD COLUMN is_recalled BOOLEAN DEFAULT FALSE;
ALTER TABLE messages ADD COLUMN recalled_at TIMESTAMP;
```

**新增API：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/messages/:id/recall | POST | 撤回消息 |

**业务规则：**
- 只能撤回2分钟内发送的消息
- 只能撤回自己发送的消息

**WebSocket消息类型：**
```javascript
{
    type: 'message_recalled',
    messageId: 123
}
```

#### 前端开发（1.5小时）

**UI交互：**
- 右键消息显示"撤回"选项
- 撤回的消息显示"消息已撤回"
- 撤回按钮只在2分钟内显示

---

### 任务5：用户头像功能（4小时）

#### 后端开发（2小时）

**数据库设计：**
```sql
-- 添加头像字段
ALTER TABLE users ADD COLUMN avatar_url TEXT;
```

**新增API：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/users/avatar | POST | 上传头像 |
| /api/users/:id/avatar | GET | 获取头像 |

#### 前端开发（2小时）

**头像显示：**
- 登录后显示用户头像
- 好友列表显示好友头像
- 聊天消息显示发送者头像

**头像上传：**
- 点击头像选择图片
- 裁剪并上传
- 默认头像处理

---

## Day 3：中低优先级功能

### 任务4：群组聊天功能（6小时）

#### 后端开发（4小时）

**数据库设计：**
```sql
-- 群组表
CREATE TABLE groups (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    creator_id INTEGER REFERENCES users(id),
    created_at TIMESTAMP DEFAULT NOW()
);

-- 群组成员表
CREATE TABLE group_members (
    id SERIAL PRIMARY KEY,
    group_id INTEGER REFERENCES groups(id),
    user_id INTEGER REFERENCES users(id),
    role VARCHAR(20) DEFAULT 'member',
    joined_at TIMESTAMP DEFAULT NOW()
);

-- 群组消息表
CREATE TABLE group_messages (
    id SERIAL PRIMARY KEY,
    group_id INTEGER REFERENCES groups(id),
    sender_id INTEGER REFERENCES users(id),
    content TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);
```

**新增API：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/groups | POST | 创建群组 |
| /api/groups | GET | 获取群组列表 |
| /api/groups/:id/members | POST | 添加成员 |
| /api/groups/:id/members/:userId | DELETE | 移除成员 |
| /api/groups/:id/messages | POST | 发送群消息 |
| /api/groups/:id/messages | GET | 获取群消息 |

#### 前端开发（2小时）

**UI设计：**
- 群组列表Tab
- 创建群组对话框
- 群组聊天界面
- 群组成员管理

---

### 任务6：系统托盘通知（2小时）

#### 前端开发（2小时）

**功能实现：**
```cpp
// 创建系统托盘图标
QSystemTrayIcon *trayIcon;

// 新消息通知
void showNotification(QString title, QString message);

// 托盘菜单
- 显示/隐藏窗口
- 在线/离线状态切换
- 退出
```

**通知触发：**
- 收到新消息时显示通知
- 收到好友请求时显示通知
- 点击通知跳转到对应聊天

---

### 任务7：消息搜索功能（3小时）

#### 后端开发（1.5小时）

**新增API：**
| API | 方法 | 功能 |
|-----|------|------|
| /api/messages/search?q=keyword | GET | 搜索消息 |

**搜索逻辑：**
```sql
SELECT * FROM messages 
WHERE (sender_id = $1 OR recipient_id = $1)
AND content ILIKE '%' || $2 || '%'
ORDER BY created_at DESC;
```

#### 前端开发（1.5小时）

**UI设计：**
- 聊天界面顶部添加搜索框
- 显示搜索结果列表
- 点击结果跳转到消息位置

---

## 每日任务清单

### Day 1

| 时间 | 任务 | 负责模块 |
|------|------|----------|
| 09:00-11:00 | 用户在线状态-后端 | 数据库+WebSocket |
| 11:00-13:00 | 用户在线状态-前端 | FriendModel+Delegate |
| 14:00-16:00 | 消息已读状态-后端 | 数据库+API |
| 16:00-18:00 | 消息已读状态-前端 | MessageModel+UI |

### Day 2

| 时间 | 任务 | 负责模块 |
|------|------|----------|
| 09:00-10:30 | 消息撤回-后端 | 数据库+API |
| 10:30-12:00 | 消息撤回-前端 | UI交互 |
| 14:00-16:00 | 用户头像-后端 | 数据库+API |
| 16:00-18:00 | 用户头像-前端 | 显示+上传 |

### Day 3

| 时间 | 任务 | 负责模块 |
|------|------|----------|
| 09:00-13:00 | 群组聊天-后端 | 数据库+API |
| 14:00-16:00 | 群组聊天-前端 | UI界面 |
| 16:00-18:00 | 系统托盘+消息搜索 | 前端 |

---

## 技术要点

### WebSocket新增消息类型

```javascript
// 用户上线/下线
{ type: 'user_status', userId: 1, isOnline: true }

// 消息已读
{ type: 'message_read', messageId: 1, readAt: '...' }

// 消息撤回
{ type: 'message_recalled', messageId: 1 }

// 群组消息
{ type: 'group_message', groupId: 1, ... }
```

### 数据库迁移脚本

```sql
-- v0.2.0 迁移脚本
-- migrations/v0.2.0.sql

-- 用户在线状态
ALTER TABLE users ADD COLUMN IF NOT EXISTS is_online BOOLEAN DEFAULT FALSE;
ALTER TABLE users ADD COLUMN IF NOT EXISTS last_seen TIMESTAMP;

-- 消息状态
ALTER TABLE messages ADD COLUMN IF NOT EXISTS is_read BOOLEAN DEFAULT FALSE;
ALTER TABLE messages ADD COLUMN IF NOT EXISTS read_at TIMESTAMP;
ALTER TABLE messages ADD COLUMN IF NOT EXISTS is_recalled BOOLEAN DEFAULT FALSE;
ALTER TABLE messages ADD COLUMN IF NOT EXISTS recalled_at TIMESTAMP;

-- 用户头像
ALTER TABLE users ADD COLUMN IF NOT EXISTS avatar_url TEXT;

-- 群组表
CREATE TABLE IF NOT EXISTS groups (...);
CREATE TABLE IF NOT EXISTS group_members (...);
CREATE TABLE IF NOT EXISTS group_messages (...);
```

---

## 验收标准

### 用户在线状态
- [ ] 用户登录后显示在线状态
- [ ] 用户退出后显示离线状态
- [ ] 好友列表实时更新在线状态

### 消息已读状态
- [ ] 发送消息显示"未读"状态
- [ ] 对方阅读后显示"已读"状态
- [ ] 好友列表显示未读消息数

### 消息撤回
- [ ] 2分钟内可撤回消息
- [ ] 撤回后显示"消息已撤回"
- [ ] 双方都能看到撤回状态

### 群组聊天
- [ ] 可创建群组
- [ ] 可添加/移除成员
- [ ] 群组消息正常收发

### 用户头像
- [ ] 可上传头像
- [ ] 头像在各处正确显示
- [ ] 默认头像处理

### 系统托盘
- [ ] 最小化到托盘
- [ ] 新消息托盘通知
- [ ] 托盘菜单功能正常

### 消息搜索
- [ ] 可搜索聊天记录
- [ ] 搜索结果正确显示
- [ ] 可跳转到消息位置

---

## 风险评估

| 风险 | 影响 | 应对措施 |
|------|------|----------|
| 群组功能复杂度高 | 可能延期 | 简化首版功能 |
| WebSocket消息类型增多 | 维护难度增加 | 统一消息格式 |
| 数据库迁移 | 数据丢失风险 | 提前备份 |

---

## 版本发布计划

| 版本 | 功能 | 预计日期 |
|------|------|----------|
| v0.2.0-alpha | 内部测试版 | Day 3 |
| v0.2.0-beta | 公开测试版 | Day 4 |
| v0.2.0 | 正式版 | Day 5 |
