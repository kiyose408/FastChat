#include "FriendModel.h"

FriendModel::FriendModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int FriendModel::rowCount(const QModelIndex &) const {
    return m_friends.size();
}

QVariant FriendModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_friends.size())
        return QVariant();

    const auto &f = m_friends.at(index.row());

    switch (role) {
    case IdRole: return f.id;
    case NicknameRole: return f.nickname;
    case IsOnlineRole: return f.isOnline;
    case LastSeenRole: return f.lastSeen;
    case UnreadCountRole: return f.unreadCount;
    case AvatarUrlRole: return f.avatarUrl;
    default: return QVariant();
    }
}

QHash<int, QByteArray> FriendModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "friendId";
    roles[NicknameRole] = "nickname";
    roles[IsOnlineRole] = "isOnline";
    roles[LastSeenRole] = "lastSeen";
    roles[UnreadCountRole] = "unreadCount";
    roles[AvatarUrlRole] = "avatarUrl";
    return roles;
}

void FriendModel::addFriend(const FriendData &friendData) {
    beginInsertRows(QModelIndex(), m_friends.size(), m_friends.size());
    m_friends.append(friendData);
    endInsertRows();
}

void FriendModel::updateOnlineStatus(int friendId, bool isOnline) {
    for (int i = 0; i < m_friends.size(); ++i) {
        if (m_friends[i].id == friendId) {
            m_friends[i].isOnline = isOnline;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {IsOnlineRole});
            qDebug() << "Updated friend" << friendId << "online status to" << isOnline;
            return;
        }
    }
}

void FriendModel::clear() {
    beginResetModel();
    m_friends.clear();
    endResetModel();
}
