#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QObject>
#include <QJsonObject>
#include <QList>
#include <QUuid>

struct QueuedMessage {
    QString id;
    QJsonObject message;
    qint64 timestamp;
    int retryCount = 0;
};

class MessageQueue : public QObject
{
    Q_OBJECT

public:
    explicit MessageQueue(int maxSize = 100, QObject* parent = nullptr);
    
    QString enqueue(const QJsonObject& message);
    QueuedMessage dequeue();
    void remove(const QString& messageId);
    void clear();
    int size() const;
    bool isEmpty() const;
    void setMaxSize(int size);
    QList<QueuedMessage> all() const;

private:
    QList<QueuedMessage> m_queue;
    int m_maxSize;
};

#endif // MESSAGEQUEUE_H
