#ifndef IAUTHSERVICE_H
#define IAUTHSERVICE_H

#include <QString>

class IAuthService{
public:
    virtual ~IAuthService() = default;
    virtual bool validateCredentials(const QString &userid, const QString &password) = 0;
};

#endif // IAUTHSERVICE_H
