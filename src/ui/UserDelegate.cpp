#include "UserDelegate.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QApplication>
#include <QMouseEvent>
#include <QInputDialog>

UserDelegate::UserDelegate(QObject *parent) 
    : QStyledItemDelegate(parent)
    , m_hoveredRow(-1)
{
}

UserDelegate::~UserDelegate()
{
}

void UserDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    
    QRect rect = option.rect;
    
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, QColor(240, 240, 240));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, QColor(248, 248, 248));
    }
    
    QString username = index.data(Qt::DisplayRole).toString();
    QString email = index.data(Qt::UserRole + 1).toString();
    int userId = index.data(Qt::UserRole).toInt();
    
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
    
    int buttonWidth = 70;
    int buttonHeight = 28;
    int noteWidth = 100;
    int noteHeight = 28;
    int spacing = 8;
    
    int rightEdge = rect.right() - 10;
    int buttonX = rightEdge - buttonWidth;
    int noteX = buttonX - spacing - noteWidth;
    int buttonY = rect.top() + (rect.height() - buttonHeight) / 2;
    
    QRect buttonRect(buttonX, buttonY, buttonWidth, buttonHeight);
    QRect noteRect(noteX, buttonY, noteWidth, noteHeight);
    
    m_buttonRects[userId] = buttonRect;
    m_noteRects[userId] = noteRect;
    
    painter->setPen(QColor(220, 220, 220));
    painter->setBrush(QColor(250, 250, 250));
    painter->drawRoundedRect(noteRect, 4, 4);
    
    QString noteText = m_notes.value(userId, "");
    QString notePlaceholder = "添加备注...";
    painter->setPen(noteText.isEmpty() ? QColor(180, 180, 180) : QColor(80, 80, 80));
    QFont noteFont = painter->font();
    noteFont.setPointSize(9);
    painter->setFont(noteFont);
    QString displayText = noteText.isEmpty() ? notePlaceholder : noteText;
    painter->drawText(noteRect.adjusted(8, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, 
                       displayText.length() > 8 ? displayText.left(8) + "..." : displayText);
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(76, 175, 80));
    painter->drawRoundedRect(buttonRect, 4, 4);
    painter->setPen(Qt::white);
    QFont btnFont = painter->font();
    btnFont.setPointSize(9);
    painter->setFont(btnFont);
    painter->drawText(buttonRect, Qt::AlignCenter, "添加");
    
    painter->restore();
}

QSize UserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 65);
}

bool UserDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();
        
        int userId = index.data(Qt::UserRole).toInt();
        
        if (m_buttonRects.contains(userId) && m_buttonRects[userId].contains(pos)) {
            QString note = m_notes.value(userId, "");
            emit addFriendRequested(userId, note);
            m_notes.remove(userId);
            return true;
        }
        
        if (m_noteRects.contains(userId) && m_noteRects[userId].contains(pos)) {
            bool ok;
            QString note = QInputDialog::getText(nullptr, "添加备注", 
                                                   "请输入好友请求备注:", 
                                                   QLineEdit::Normal, 
                                                   m_notes.value(userId, ""), &ok);
            if (ok) {
                m_notes[userId] = note;
            }
            return true;
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
