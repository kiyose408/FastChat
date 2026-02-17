#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct MessageData {
    bool isSelf;
    QString text;
    QString time;
    QString messageType;
    QString fileUrl;
    QString fileName;
};

class MessageModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        IsSelfRole = Qt::UserRole + 1,
        TextRole,
        TimeRole,
        MessageTypeRole,
        FileUrlRole,
        FileNameRole
    };

    explicit MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(const MessageData &msg);
    void clear();

private:
    QVector<MessageData> m_messages;
};

#endif // MESSAGEMODEL_H
