// SessionManager.cpp
// 实现了会话管理器类 SessionManager 的功能，用于存储和管理用户登录状态信息（如 token、用户ID、用户名），
// 并提供持久化保存和加载功能。

#include "SessionManager.h" // 包含头文件声明
#include <QSettings>        // 用于读写应用程序设置（配置文件）
#include <QJsonDocument>    // 用于处理 JSON 数据（虽然此文件未使用，但包含在内）
#include <QJsonObject>      // 用于处理 JSON 对象（虽然此文件未使用，但包含在内）

// 静态单例方法：获取 SessionManager 的唯一实例
// 使用静态局部变量确保只创建一次实例，并保证线程安全（C++11 及以上标准）
SessionManager& SessionManager::instance()
{
    static SessionManager inst; // 局部静态变量，首次调用时初始化
    return inst;                // 返回引用
}

// 获取当前存储的 token（通常用于认证）
QString SessionManager::token() const { return m_token; }

// 获取当前存储的用户 ID
int SessionManager::userId() const { return m_userId; }

// 获取当前存储的用户名
QString SessionManager::username() const { return m_username; }

// 设置新的 token 值
// 如果新值与当前值不同，则更新并发出信号通知监听者
void SessionManager::setToken(const QString &token)
{
    if (m_token != token) { // 检查是否发生变化
        m_token = token;    // 更新内部存储的 token
        emit tokenChanged(); // 发出信号，通知其他组件 token 已改变
    }
}

// 设置用户信息（用户 ID 和用户名）
// 如果新值与当前值不同，则更新并发出信号通知监听者
void SessionManager::setUserInfo(int userId, const QString &username)
{
    if (m_userId != userId || m_username != username) { // 检查任一值是否发生变化
        m_userId = userId;     // 更新内部存储的用户 ID
        m_username = username; // 更新内部存储的用户名
        emit userChanged();    // 发出信号，通知其他组件用户信息已改变
    }
}

// 将当前的会话信息（token、用户ID、用户名）保存到系统设置中
// 使用 QSettings 将数据写入配置文件（通常是 .ini 文件或注册表）
void SessionManager::save()
{
    QSettings settings; // 创建 QSettings 对象，使用默认的组织名和应用名作为组名
    settings.setValue("Session/token", m_token);       // 保存 token
    settings.setValue("Session/userId", m_userId);     // 保存用户 ID
    settings.setValue("Session/username", m_username); // 保存用户名
}

// 从系统设置中加载会话信息
// 从配置文件中读取之前保存的 token、用户ID、用户名，并更新内部状态
// 如果成功加载了 token，则认为用户已登录，发出相应信号
void SessionManager::load()
{
    QSettings settings; // 创建 QSettings 对象
    m_token = settings.value("Session/token").toString();         // 读取 token，默认为空字符串
    m_userId = settings.value("Session/userId", 0).toInt();      // 读取用户 ID，默认为 0
    m_username = settings.value("Session/username").toString();   // 读取用户名，默认为空字符串

    // 如果 token 不为空，则表明之前有有效的登录会话
    if (!m_token.isEmpty()) {
        emit tokenChanged(); // 通知 token 已改变（可能用于 UI 更新）
        emit userChanged();  // 通知用户信息已改变（可能用于 UI 更新）
        emit loggedIn();     // 通知应用程序用户已登录（可由主窗口等监听）
    }
}

// 清除当前的会话信息
// 从系统设置中删除所有与 Session 相关的键值对，并将内部成员变量清空
// 同时发出相应的信号，通知监听者会话已结束
void SessionManager::clear()
{
    QSettings settings; // 创建 QSettings 对象
    settings.remove("Session"); // 删除 "Session" 组下的所有键值对

    m_token.clear();     // 清空 token 字符串
    m_userId = 0;        // 将用户 ID 设为默认值 0
    m_username.clear();  // 清空用户名字符串

    emit tokenChanged(); // 通知 token 已改变（通常为清空）
    emit userChanged();  // 通知用户信息已改变（通常为清空）
    emit loggedOut();    // 通知应用程序用户已登出（可由主窗口等监听）
}
