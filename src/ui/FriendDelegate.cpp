#include "FriendDelegate.h"
#include <QPainter>
#include <QFontMetrics>

// =============== 颜色主题配置区（便于后期维护和换肤） ===============
static const QColor SELECTED_BACKGROUND_COLOR(0x34, 0x98, 0xDB); // #3498DB - 选中项背景色
static const QColor AVATAR_BACKGROUND_COLOR(0x4A, 0x90, 0xE2);   // #4A90E2 - 头像背景色
static const QColor TEXT_COLOR_BLACK(Qt::black);                  // 黑色文字
static const QColor ONLINE_STATUS_COLOR(Qt::green);               // 在线状态绿色
static const QColor OFFLINE_STATUS_COLOR(Qt::gray);               // 离线状态灰色
static const QColor UNREAD_DOT_COLOR(Qt::red);                    // 未读红点颜色
static const QColor DIVIDER_COLOR(0xEE, 0xEE, 0xEE);              // 分割线颜色
// =======================================================

void FriendDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    // 背景
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    else
        painter->fillRect(option.rect, Qt::white);

    int id = index.data(FriendModel::IdRole).toInt();
    QString nickname = index.data(FriendModel::NicknameRole).toString();
    bool online = index.data(FriendModel::OnlineRole).toBool();
    int unread = index.data(FriendModel::UnreadRole).toInt();

    // 头像区域
    QRect avatarRect = option.rect.adjusted(10, 10, 0, -10);
    avatarRect.setWidth(40); avatarRect.setHeight(40);

    // 绘制头像（圆形背景 + 首字母）
    painter->setBrush(AVATAR_BACKGROUND_COLOR);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(avatarRect);
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 16, QFont::Bold));
    painter->drawText(avatarRect, Qt::AlignCenter, nickname.left(1).toUpper());

    // 昵称
    QRect nameRect = option.rect.adjusted(60, 10, -10, -30);
    painter->setPen(TEXT_COLOR_BLACK);
    painter->setFont(QFont("Arial", 12, QFont::Bold));
    painter->drawText(nameRect, nickname);

    // 在线状态
    painter->setPen(online ? ONLINE_STATUS_COLOR : OFFLINE_STATUS_COLOR);
    painter->setFont(QFont("Arial", 9));
    QRect statusRect = nameRect.adjusted(0, 20, 0, 0);
    painter->drawText(statusRect, online ? "● 在线" : "○ 离线");

    // 未读红点
    if (unread > 0) {
        QRect dotRect(option.rect.right() - 40, option.rect.top() + 15, 20, 20);
        painter->setBrush(UNREAD_DOT_COLOR);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(dotRect);
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(dotRect, Qt::AlignCenter, QString::number(unread));
    }

    // 分割线
    painter->setPen(DIVIDER_COLOR);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    painter->restore();
}

QSize FriendDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
    return QSize(200, 60);
}
