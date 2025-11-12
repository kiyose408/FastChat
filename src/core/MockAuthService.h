#ifndef MOCKAUTHSERVICE_H
#define MOCKAUTHSERVICE_H

#include "IAuthService.h"
#include <QMap>
class MockAuthService : public IAuthService
{
public:
    MockAuthService();
    bool validateCredentials(const QString &userid, const QString &password)override;

private:
    QMap<QString, QString> m_validUsers;
};

#endif // MOCKAUTHSERVICE_H
