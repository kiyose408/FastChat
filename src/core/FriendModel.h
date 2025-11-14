#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <QAbstractListModel>  // Qt 抽象列表模型基类，用于提供数据给视图（如 ListView）
#include <QVector>             // Qt 的动态数组容器，用于存储好友数据

// 定义一个结构体，用于表示单个好友的信息
struct FriendData {
    int id;                    // 好友的唯一标识符
    QString nickname;          // 好友的昵称
    bool isOnline;             // 好友是否在线
    int unreadCount;           // 该好友发来的未读消息数量
};

// FriendModel 继承自 QAbstractListModel，用于在 Qt Quick 或 QWidget 视图中展示好友列表
class FriendModel : public QAbstractListModel {
    Q_OBJECT  // 必须包含此宏，以支持 Qt 的元对象系统（如信号/槽、属性等）

public:
    // 自定义角色枚举，用于在 QML 或视图中通过角色名访问数据
    // Qt::UserRole 是用户自定义角色的起始值，后续依次递增
    enum Role {
        IdRole = Qt::UserRole + 1,     // 对应好友 ID
        NicknameRole,                  // 对应昵称
        OnlineRole,                    // 对应在线状态
        UnreadRole                     // 对应未读消息数
    };

    // 构造函数，可选传入父对象（用于内存管理）
    explicit FriendModel(QObject *parent = nullptr);

    // 重写基类方法：返回模型中的行数（即好友数量）
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 重写基类方法：根据索引和角色返回对应的数据
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 重写基类方法：将自定义角色映射为字符串名称，便于在 QML 中使用（如 "id", "nickname"）
    QHash<int, QByteArray> roleNames() const override;

    // 向模型中添加一个好友
    void addFriend(const FriendData &f);

    // 清空所有好友数据
    void clear();

private:
    QVector<FriendData> m_friends;  // 存储所有好友数据的容器
};

#endif // FRIENDMODEL_H
