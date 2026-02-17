#ifndef USERDELEGATE_H
#define USERDELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QLineEdit>

class UserDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit UserDelegate(QObject *parent = nullptr);
    ~UserDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void addFriendRequested(int userId, const QString& note);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    mutable QMap<int, QRect> m_buttonRects;
    mutable QMap<int, QRect> m_noteRects;
    mutable QMap<int, QString> m_notes;
    mutable int m_hoveredRow;
};

#endif // USERDELEGATE_H
