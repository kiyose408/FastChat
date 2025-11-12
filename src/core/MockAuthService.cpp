#include "MockAuthService.h"

MockAuthService::MockAuthService()
{
    // 虚拟用户数据（开发阶段用）
    m_validUsers["admin"] = "123456";
    m_validUsers["user"] = "password";
    m_validUsers["test"] = "test123";
}

bool MockAuthService::validateCredentials(const QString &userid, const QString &password)
{
    if (m_validUsers.contains(userid)) {
        return m_validUsers[userid] == password;
    }
    return false;
}
