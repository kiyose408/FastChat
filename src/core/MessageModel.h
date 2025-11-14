#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>  // Qt 抽象列表模型基类，用于向视图（如 ListView）提供数据
#include <QVector>             // Qt 动态数组容器，用于存储消息列表

// 定义一条聊天消息的数据结构
struct MessageData {
    bool isSelf;      // 标识该消息是否由当前用户发送（true 表示自己，false 表示对方）
    QString text;     // 消息的文本内容
    QString time;     // 消息的时间戳（通常格式如 "14:30" 或 "2025-11-14 14:30"）
    // 注：也可使用 QDateTime，但 QString 更便于在 QML 中直接显示
};

// MessageModel 继承自 QAbstractListModel，用于在 Qt Widgets 或 QML 中展示聊天消息列表
class MessageModel : public QAbstractListModel {
    Q_OBJECT  // 必须包含此宏，以启用 Qt 的元对象系统（支持信号/槽、属性、QML 集成等）

public:
    // 自定义角色枚举，用于在 QML 或视图中通过语义化名称访问数据字段
    // Qt::UserRole 是用户自定义角色的起始值，后续依次递增
    enum Role {
        IsSelfRole = Qt::UserRole + 1,  // 对应 isSelf 字段（bool）
        TextRole,                       // 对应 text 字段（QString）
        TimeRole                        // 对应 time 字段（QString）
    };

    // 构造函数，可选传入父对象（用于内存管理，如被 QObject 父对象自动析构）
    explicit MessageModel(QObject *parent = nullptr);

    // 重写基类方法：返回模型中的行数（即消息总数）
    // parent 参数在此列表模型中通常未使用，但需保留以符合接口规范
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 重写基类方法：根据模型索引（index）和请求的角色（role）返回对应的数据
    // 这是视图或 QML 获取每条消息内容的核心接口
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 重写基类方法：将自定义角色映射为字符串名称
    // 例如：IsSelfRole → "isSelf"，这样在 QML 的 delegate 中可以直接使用 `isSelf`
    QHash<int, QByteArray> roleNames() const override;

    // 向消息列表末尾添加一条新消息
    // 内部会自动通知视图更新（通过 beginInsertRows / endInsertRows）
    void addMessage(const MessageData &msg);

    // 清空所有消息
    // 适用于切换聊天对象或重新加载会话时
    void clear();

private:
    QVector<MessageData> m_messages;  // 存储所有聊天消息的内部容器
};

#endif // MESSAGEMODEL_H
