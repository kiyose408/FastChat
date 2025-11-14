#include "MessageModel.h"

// 构造函数：调用基类 QAbstractListModel 的构造函数，并传入父对象（用于内存管理）
MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // m_messages 默认为空，无需额外初始化
}

// 返回模型中的行数（即消息列表的长度）
// parent 参数在此列表模型中通常无用（树形模型才需要），但需保留以符合接口要求
int MessageModel::rowCount(const QModelIndex &) const {
    return m_messages.size();  // 直接返回内部容器的消息数量
}

// 根据给定的模型索引（index）和数据角色（role）返回对应的数据
// 这是视图（如 ListView）或 QML 获取每条消息内容的核心方法
QVariant MessageModel::data(const QModelIndex &index, int role) const {
    // 检查索引是否有效，以及行号是否越界
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();  // 无效时返回空 QVariant

    // 获取对应行的消息数据（使用 at() 更安全，不会修改容器）
    const auto &m = m_messages.at(index.row());

    // 根据请求的角色（role）返回相应的字段
    switch (role) {
    case IsSelfRole: return m.isSelf;   // 是否是自己发送的消息（bool）
    case TextRole:   return m.text;     // 消息文本内容（QString）
    case TimeRole:   return m.time;     // 消息时间（QDateTime 或 QString，取决于 MessageData 定义）
    default:         return QVariant(); // 不支持的角色返回空值
    }
}

// 返回角色（role）到字符串名称的映射
// 此方法在 QML 中尤其重要，使得可以在委托（delegate）中通过属性名（如 "text"）访问数据
QHash<int, QByteArray> MessageModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IsSelfRole] = "isSelf";  // 在 QML 中可通过 isSelf 判断消息方向
    roles[TextRole]   = "text";    // 显示消息内容
    roles[TimeRole]   = "time";    // 显示时间戳
    return roles;
}

// 向模型末尾添加一条新消息
// 注意：必须使用 beginInsertRows / endInsertRows 包裹数据变更，
// 以通知视图有新行插入，从而正确更新 UI（如自动滚动到底部）
void MessageModel::addMessage(const MessageData &msg) {
    // 告诉模型即将在 [size, size] 范围插入一行（即末尾）
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(msg);  // 实际添加消息数据
    endInsertRows();         // 通知模型插入完成
}

// 清空所有消息
// 当模型结构发生大规模变化（如切换聊天对象）时，应使用 beginResetModel / endResetModel
void MessageModel::clear() {
    beginResetModel();   // 通知视图：模型将重置，当前所有索引将失效
    m_messages.clear();  // 清空内部消息列表
    endResetModel();     // 通知视图：模型已重置，可重新获取数据
}
