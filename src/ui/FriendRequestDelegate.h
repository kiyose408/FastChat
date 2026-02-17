#ifndef FRIENDREQUESTDELEGATE_H
#define FRIENDREQUESTDELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QPushButton>
#include <QPainter>

class FriendRequestDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FriendRequestDelegate(QObject *parent = nullptr);
    ~FriendRequestDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void acceptRequest(int userId);
    void rejectRequest(int userId);

private:
    QRect m_acceptButtonRect;
    QRect m_rejectButtonRect;
    int m_hoveredRow;
};

#endif // FRIENDREQUESTDELEGATE_H
