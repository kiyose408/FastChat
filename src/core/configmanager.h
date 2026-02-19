#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <QString>
#include <QSettings>
class ConfigManager
{
public:
    static ConfigManager& instance();

    //获取/设置服务器地址  Get/Set the address of server
    QString serverAddress() const;
    void setServerAddress(const QString& addr);

    //窗口大小记忆
    QSize windowSize() const;
    void setWindowSize(const QSize& size);

private:
    ConfigManager();
    mutable QSettings m_settings{"KIYOSE","FastChat"};  //自动选择存储位置
};

#endif // CONFIGMANAGER_H
