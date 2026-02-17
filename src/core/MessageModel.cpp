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
    case IsSelfRole: return m.isSelf;
    case TextRole:   return m.text;
    case TimeRole:   return m.time;
    case MessageTypeRole: return m.messageType;
    case FileUrlRole: return m.fileUrl;
    case FileNameRole: return m.fileName;
    default:         return QVariant();
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IsSelfRole] = "isSelf";
    roles[TextRole]   = "text";
    roles[TimeRole]   = "time";
    roles[MessageTypeRole] = "messageType";
    roles[FileUrlRole] = "fileUrl";
    roles[FileNameRole] = "fileName";
    return roles;
}

void MessageModel::addMessage(const MessageData &msg) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(msg);
    endInsertRows();
}

void MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}
