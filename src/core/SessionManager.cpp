#include "SessionManager.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>

SessionManager& SessionManager::instance()
{
    static SessionManager inst;
    return inst;
}

QString SessionManager::token() const { return m_token; }
int SessionManager::userId() const { return m_userId; }
QString SessionManager::username() const { return m_username; }

void SessionManager::setToken(const QString &token)
{
    if (m_token != token) {
        m_token = token;
        emit tokenChanged();
    }
}

void SessionManager::setUserInfo(int userId, const QString &username)
{
    if (m_userId != userId || m_username != username) {
        m_userId = userId;
        m_username = username;
        emit userChanged();
    }
}

void SessionManager::save()
{
    QSettings settings;
    settings.setValue("Session/token", m_token);
    settings.setValue("Session/userId", m_userId);
    settings.setValue("Session/username", m_username);
}

void SessionManager::load()
{
    QSettings settings;
    m_token = settings.value("Session/token").toString();
    m_userId = settings.value("Session/userId", 0).toInt();
    m_username = settings.value("Session/username").toString();
    if (!m_token.isEmpty()) {
        emit tokenChanged();
        emit userChanged();
        emit loggedIn();
    }
}

void SessionManager::clear()
{
    QSettings settings;
    settings.remove("Session");
    m_token.clear();
    m_userId = 0;
    m_username.clear();
    emit tokenChanged();
    emit userChanged();
    emit loggedOut();
}
