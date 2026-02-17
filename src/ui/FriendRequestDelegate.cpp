#include "FriendRequestDelegate.h"
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>

FriendRequestDelegate::FriendRequestDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_hoveredRow(-1)
{
}

FriendRequestDelegate::~FriendRequestDelegate()
{
}

void FriendRequestDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    
    QRect rect = option.rect;
    
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, QColor(240, 240, 240));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, QColor(248, 248, 248));
    }
    
    int userId = index.data(Qt::UserRole).toInt();
    QString username = index.data(Qt::DisplayRole).toString();
    QString email = index.data(Qt::UserRole + 1).toString();
    QString note = index.data(Qt::UserRole + 2).toString();
    
    int avatarSize = 40;
    int avatarX = rect.left() + 15;
    int avatarY = rect.top() + (rect.height() - avatarSize) / 2;
    
    painter->setBrush(QColor(200, 200, 200));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(avatarX, avatarY, avatarSize, avatarSize);
    
    painter->setPen(Qt::white);
    QFont avatarFont = painter->font();
    avatarFont.setPointSize(16);
    avatarFont.setBold(true);
    painter->setFont(avatarFont);
    QString initial = username.isEmpty() ? "?" : username.left(1).toUpper();
    painter->drawText(QRect(avatarX, avatarY, avatarSize, avatarSize), Qt::AlignCenter, initial);
    
    int textLeft = avatarX + avatarSize + 15;
    
    painter->setPen(QColor(50, 50, 50));
    QFont usernameFont = painter->font();
    usernameFont.setPointSize(11);
    usernameFont.setBold(true);
    painter->setFont(usernameFont);
    painter->drawText(textLeft, rect.top() + 22, username);
    
    painter->setPen(QColor(130, 130, 130));
    QFont emailFont = painter->font();
    emailFont.setPointSize(9);
    emailFont.setBold(false);
    painter->setFont(emailFont);
    painter->drawText(textLeft, rect.top() + 42, email);
    
    if (!note.isEmpty()) {
        painter->setPen(QColor(100, 100, 100));
        QFont noteFont = painter->font();
        noteFont.setPointSize(9);
        noteFont.setItalic(true);
        painter->setFont(noteFont);
        QString noteText = "备注: " + note;
        painter->drawText(textLeft, rect.top() + 60, noteText);
    }
    
    int buttonWidth = 60;
    int buttonHeight = 28;
    int buttonY = rect.top() + (rect.height() - buttonHeight) / 2;
    
    QRect acceptRect(rect.right() - 2 * buttonWidth - 20, buttonY, buttonWidth, buttonHeight);
    QRect rejectRect(rect.right() - buttonWidth - 10, buttonY, buttonWidth, buttonHeight);
    
    const_cast<FriendRequestDelegate*>(this)->m_acceptButtonRect = acceptRect;
    const_cast<FriendRequestDelegate*>(this)->m_rejectButtonRect = rejectRect;
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(76, 175, 80));
    painter->drawRoundedRect(acceptRect, 4, 4);
    painter->setPen(Qt::white);
    QFont btnFont = painter->font();
    btnFont.setPointSize(9);
    btnFont.setItalic(false);
    painter->setFont(btnFont);
    painter->drawText(acceptRect, Qt::AlignCenter, "接受");
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(244, 67, 54));
    painter->drawRoundedRect(rejectRect, 4, 4);
    painter->setPen(Qt::white);
    painter->drawText(rejectRect, Qt::AlignCenter, "拒绝");
    
    painter->restore();
}

QSize FriendRequestDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString note = index.data(Qt::UserRole + 2).toString();
    int height = note.isEmpty() ? 65 : 80;
    return QSize(option.rect.width(), height);
}

bool FriendRequestDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();
        
        int userId = index.data(Qt::UserRole).toInt();
        
        if (m_acceptButtonRect.contains(pos)) {
            emit acceptRequest(userId);
            return true;
        } else if (m_rejectButtonRect.contains(pos)) {
            emit rejectRequest(userId);
            return true;
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
