#include "RetryPolicy.h"
#include <QDebug>
#include <QtMath>

RetryPolicy::RetryPolicy(const RetryConfig& config, QObject* parent)
    : QObject(parent)
    , m_config(config)
{
}

bool RetryPolicy::shouldRetry(QNetworkReply::NetworkError error, int statusCode) const
{
    if (!canRetry(0)) {
        return false;
    }
    
    if (isRetryableError(error)) {
        return true;
    }
    
    if (isRetryableStatus(statusCode)) {
        return true;
    }
    
    return false;
}

int RetryPolicy::calculateDelay(int retryCount) const
{
    if (retryCount <= 0) {
        return m_config.baseDelayMs;
    }
    
    double delay = m_config.baseDelayMs * 
                   qPow(m_config.backoffMultiplier, retryCount - 1);
    
    return qMin(static_cast<int>(delay), m_config.maxDelayMs);
}

bool RetryPolicy::canRetry(int retryCount) const
{
    return retryCount < m_config.maxRetries;
}

bool RetryPolicy::isRetryableStatus(int statusCode) const
{
    return m_config.retryableStatusCodes.contains(statusCode);
}

bool RetryPolicy::isRetryableError(QNetworkReply::NetworkError error) const
{
    switch (error) {
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::TemporaryNetworkFailureError:
        case QNetworkReply::NetworkSessionFailedError:
        case QNetworkReply::UnknownNetworkError:
        case QNetworkReply::ServiceUnavailableError:
            return true;
        default:
            return false;
    }
}

void RetryPolicy::setConfig(const RetryConfig& config)
{
    m_config = config;
}

RetryConfig RetryPolicy::config() const
{
    return m_config;
}
