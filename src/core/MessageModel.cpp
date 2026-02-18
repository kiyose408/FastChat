#include "MessageModel.h"

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex &) const {
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();

    const auto &m = m_messages.at(index.row());

    switch (role) {
    case MessageIdRole: return m.messageId;
    case IsSelfRole: return m.isSelf;
    case TextRole:   return m.text;
    case TimeRole:   return m.time;
    case MessageTypeRole: return m.messageType;
    case FileUrlRole: return m.fileUrl;
    case FileNameRole: return m.fileName;
    case IsReadRole: return m.isRead;
    case IsRecalledRole: return m.isRecalled;
    default:         return QVariant();
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[MessageIdRole] = "messageId";
    roles[IsSelfRole] = "isSelf";
    roles[TextRole]   = "text";
    roles[TimeRole]   = "time";
    roles[MessageTypeRole] = "messageType";
    roles[FileUrlRole] = "fileUrl";
    roles[FileNameRole] = "fileName";
    roles[IsReadRole] = "isRead";
    roles[IsRecalledRole] = "isRecalled";
    return roles;
}

void MessageModel::addMessage(const MessageData &msg) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(msg);
    endInsertRows();
}

void MessageModel::markAllAsRead() {
    for (int i = 0; i < m_messages.size(); ++i) {
        if (!m_messages[i].isSelf && !m_messages[i].isRead) {
            m_messages[i].isRead = true;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {IsReadRole});
        }
    }
}

void MessageModel::markSentAsRead() {
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].isSelf && !m_messages[i].isRead) {
            m_messages[i].isRead = true;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {IsReadRole});
        }
    }
}

void MessageModel::recallMessage(int messageId) {
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].messageId == messageId) {
            m_messages[i].isRecalled = true;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {IsRecalledRole, TextRole});
            break;
        }
    }
}

void MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}
