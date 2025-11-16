#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>

class SessionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)
    Q_PROPERTY(int userId READ userId NOTIFY userChanged)
    Q_PROPERTY(QString username READ username NOTIFY userChanged)

public:
    static SessionManager& instance();

    // Getters
    QString token() const;
    int userId() const;
    QString username() const;

    // Setters (called after login)
    void setToken(const QString& token);
    void setUserInfo(int userId, const QString& username);

    // 持久化到本地（可选）
    void save();
    void load();

    // 清除登录信息
    void clear();

signals:
    void tokenChanged();
    void userChanged();
    void loggedIn();
    void loggedOut();

private:
    SessionManager() = default;
    QString m_token;
    int m_userId = 0;
    QString m_username;
};

#endif // SESSIONMANAGER_H
