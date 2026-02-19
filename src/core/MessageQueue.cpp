#include "MessageQueue.h"
#include <QDateTime>

MessageQueue::MessageQueue(int maxSize, QObject* parent)
    : QObject(parent)
    , m_maxSize(maxSize)
{
}

QString MessageQueue::enqueue(const QJsonObject& message)
{
    if (m_queue.size() >= m_maxSize) {
        m_queue.removeFirst();
    }
    
    QueuedMessage msg;
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.message = message;
    msg.timestamp = QDateTime::currentMSecsSinceEpoch();
    msg.retryCount = 0;
    
    m_queue.append(msg);
    return msg.id;
}

QueuedMessage MessageQueue::dequeue()
{
    if (m_queue.isEmpty()) {
        return QueuedMessage();
    }
    
    return m_queue.takeFirst();
}

void MessageQueue::remove(const QString& messageId)
{
    for (int i = 0; i < m_queue.size(); ++i) {
        if (m_queue[i].id == messageId) {
            m_queue.removeAt(i);
            break;
        }
    }
}

void MessageQueue::clear()
{
    m_queue.clear();
}

int MessageQueue::size() const
{
    return m_queue.size();
}

bool MessageQueue::isEmpty() const
{
    return m_queue.isEmpty();
}

void MessageQueue::setMaxSize(int size)
{
    m_maxSize = size;
    while (m_queue.size() > m_maxSize) {
        m_queue.removeFirst();
    }
}

QList<QueuedMessage> MessageQueue::all() const
{
    return m_queue;
}
