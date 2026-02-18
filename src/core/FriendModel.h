#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct FriendData {
    int id;
    QString nickname;
    bool isOnline;
    QString lastSeen;
    int unreadCount;
    QString avatarUrl;
};

class FriendModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        NicknameRole,
        IsOnlineRole,
        LastSeenRole,
        UnreadCountRole,
        AvatarUrlRole
    };

    explicit FriendModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addFriend(const FriendData &friendData);
    void updateOnlineStatus(int friendId, bool isOnline);
    void clear();

private:
    QVector<FriendData> m_friends;
};

#endif // FRIENDMODEL_H
