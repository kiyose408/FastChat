#ifndef RETRYPOLICY_H
#define RETRYPOLICY_H

#include <QObject>
#include <QNetworkReply>
#include <QList>

struct RetryConfig {
    int maxRetries = 3;
    int baseDelayMs = 1000;
    int maxDelayMs = 10000;
    double backoffMultiplier = 2.0;
    QList<int> retryableStatusCodes;
    
    RetryConfig() {
        retryableStatusCodes = {408, 429, 500, 502, 503, 504};
    }
};

class RetryPolicy : public QObject
{
    Q_OBJECT

public:
    explicit RetryPolicy(const RetryConfig& config = RetryConfig(), QObject* parent = nullptr);
    
    bool shouldRetry(QNetworkReply::NetworkError error, int statusCode) const;
    int calculateDelay(int retryCount) const;
    bool canRetry(int retryCount) const;
    bool isRetryableStatus(int statusCode) const;
    bool isRetryableError(QNetworkReply::NetworkError error) const;
    
    void setConfig(const RetryConfig& config);
    RetryConfig config() const;

private:
    RetryConfig m_config;
};

#endif // RETRYPOLICY_H
