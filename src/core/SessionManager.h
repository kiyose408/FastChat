#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject> // 包含 Qt 的核心对象类，用于继承 QObject
#include <QJsonObject> // 包含 QJsonObject 类，用于处理 JSON 对象
#include <QJsonDocument> // 包含 QJsonDocument 类，用于处理 JSON 文档

// 定义 SessionManager 类，继承自 QObject
// 使用 Q_OBJECT 宏以便支持 Qt 的元对象系统（信号槽、属性等）
class SessionManager : public QObject
{
    Q_OBJECT // Qt 元对象系统宏，启用信号槽和属性系统
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged) // 定义一个名为 token 的只读属性
    Q_PROPERTY(int userId READ userId NOTIFY userChanged) // 定义一个名为 userId 的只读属性
    Q_PROPERTY(QString username READ username NOTIFY userChanged) // 定义一个名为 username 的只读属性

public:
    // 获取 SessionManager 单例实例的静态方法
    static SessionManager& instance();

    // Getter 方法：获取 token 值
    QString token() const;
    // Getter 方法：获取用户 ID 值
    int userId() const;
    // Getter 方法：获取用户名值
    QString username() const;

    // Setter 方法：设置 token 值（通常在登录后调用）
    void setToken(const QString& token);
    // Setter 方法：设置用户信息（用户 ID 和用户名）（通常在登录后调用）
    void setUserInfo(int userId, const QString& username);

    // 持久化方法：将当前会话信息保存到本地存储（如文件或 QSettings）
    void save();
    // 加载方法：从本地存储加载会话信息
    void load();

    // 清除方法：清除当前的登录状态信息
    void clear();

signals:
    // 当 token 发生变化时发出的信号
    void tokenChanged();
    // 当用户信息（userId 或 username）发生变化时发出的信号
    void userChanged();
    // 当用户成功登录时发出的信号
    void loggedIn();
    // 当用户登出时发出的信号
    void loggedOut();

private:
    // 构造函数设为私有，确保只能通过 instance() 方法获取实例
    SessionManager() = default;
    // 私有成员变量：存储访问令牌
    QString m_token;
    // 私有成员变量：存储用户 ID
    int m_userId = 0; // 初始化为 0
    // 私有成员变量：存储用户名
    QString m_username;
};

#endif // SESSIONMANAGER_H
