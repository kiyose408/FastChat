#include "FriendModel.h"

// 构造函数：调用基类 QAbstractListModel 的构造函数，并传入父对象（用于内存管理）
FriendModel::FriendModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // 此处无需额外初始化，m_friends 默认为空
}

// 返回模型中的行数（即好友列表的长度）
// parent 参数在此列表模型中通常无用（树形模型才需要），但需保留以符合接口要求
int FriendModel::rowCount(const QModelIndex &) const {
    return m_friends.size();  // 直接返回内部容器的元素个数
}

// 根据给定的模型索引（index）和数据角色（role）返回对应的数据
// 这是视图（如 ListView）或 QML 获取每一项内容的核心方法
QVariant FriendModel::data(const QModelIndex &index, int role) const {
    // 检查索引是否有效，以及行号是否越界
    if (!index.isValid() || index.row() >= m_friends.size())
        return QVariant();  // 无效时返回空 QVariant

    // 获取对应行的好友数据（使用 at() 更安全，不会修改容器）
    const auto &f = m_friends.at(index.row());

    // 根据请求的角色（role）返回相应的字段
    switch (role) {
    case IdRole:        return f.id;           // 返回好友 ID
    case NicknameRole:  return f.nickname;     // 返回昵称（QString 自动转为 QVariant）
    case OnlineRole:    return f.isOnline;     // 返回在线状态（bool）
    case UnreadRole:    return f.unreadCount;  // 返回未读消息数（int）
    default:            return QVariant();     // 不支持的角色返回空值
    }
}

// 返回角色（role）到字符串名称的映射
// 此方法在 QML 中尤其重要，使得可以在委托（delegate）中通过属性名（如 nickname）访问数据
QHash<int, QByteArray> FriendModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "friendId";      // 在 QML 中可通过 friendId 访问
    roles[NicknameRole] = "nickname";
    roles[OnlineRole] = "isOnline";
    roles[UnreadRole] = "unreadCount";
    return roles;
}

// 向模型末尾添加一个好友
// 注意：必须使用 beginInsertRows / endInsertRows 包裹数据变更，
// 以通知视图有新行插入，从而正确更新 UI
void FriendModel::addFriend(const FriendData &f) {
    // 告诉模型即将在 [size, size] 范围插入一行（即末尾）
    beginInsertRows(QModelIndex(), m_friends.size(), m_friends.size());
    m_friends.append(f);  // 实际添加数据
    endInsertRows();      // 通知模型插入完成
}

// 清空所有好友数据
// 当模型结构发生大规模变化（如全部清空）时，应使用 beginResetModel / endResetModel
void FriendModel::clear() {
    beginResetModel();   // 通知视图：模型将重置，当前所有索引将失效
    m_friends.clear();   // 清空内部数据
    endResetModel();     // 通知视图：模型已重置，可重新获取数据
}
