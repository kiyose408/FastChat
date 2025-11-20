#include "configmanager.h"
#include <QSize>

ConfigManager& ConfigManager::instance(){
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager() = default;

QString ConfigManager::serverAddress() const{
    return m_settings.value("Network/SeverAddress", "127.0.0.1:8080").toString();
}

void ConfigManager::setServerAddress(const QSize &addr)
{
    m_settings.setValue("Network/ServerAddress", addr);
}

QSize ConfigManager::windowSize() const
{
    return m_settings.value("UI/WindowSize",QSize(580,800)).toSize();
}

void ConfigManager::setWindowSize(const QSize &size)
{
    m_settings.setValue("UI/WindowSize",size);
}

