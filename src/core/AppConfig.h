#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

class AppConfig
{
public:
    static QString serverHost() { return "localhost"; }
    static int serverPort() { return 3000; }
    static QString serverBaseUrl() { return "http://localhost:3000"; }
    static QString wsBaseUrl() { return "ws://localhost:3000"; }
    
    static QString avatarBaseUrl() { return serverBaseUrl(); }
    static QString uploadUrl() { return serverBaseUrl() + "/api/upload"; }
    
private:
    AppConfig() = delete;
};

#endif // APPCONFIG_H
