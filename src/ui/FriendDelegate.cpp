#include "FriendDelegate.h"
#include "core/FriendModel.h"
#include "core/AppConfig.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QCache>

static const QColor SELECTED_BACKGROUND_COLOR(0x34, 0x98, 0xDB);
static const QColor AVATAR_BACKGROUND_COLOR(0x4A, 0x90, 0xE2);
static const QColor TEXT_COLOR_BLACK(Qt::black);
static const QColor ONLINE_STATUS_COLOR(0x2E, 0xCC, 0x71);
static const QColor OFFLINE_STATUS_COLOR(0x95, 0xA5, 0xA6);
static const QColor UNREAD_DOT_COLOR(0xE7, 0x4C, 0x3C);
static const QColor DIVIDER_COLOR(0xEE, 0xEE, 0xEE);

static QNetworkAccessManager* networkManager() {
    static QNetworkAccessManager* manager = new QNetworkAccessManager();
    return manager;
}

static QCache<QString, QPixmap>& avatarCache() {
    static QCache<QString, QPixmap> cache(50);
    return cache;
}

void FriendDelegate::loadAvatar(const QString& url) const {
    if (url.isEmpty() || avatarCache().contains(url)) return;
    
    QString fullUrl = AppConfig::avatarBaseUrl() + url;
    QNetworkReply* reply = networkManager()->get(QNetworkRequest(QUrl(fullUrl)));
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap* pixmap = new QPixmap();
            if (pixmap->loadFromData(data)) {
                avatarCache().insert(url, pixmap);
            } else {
                delete pixmap;
            }
        }
        reply->deleteLater();
    });
}

void FriendDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, SELECTED_BACKGROUND_COLOR);
    } else {
        painter->fillRect(option.rect, Qt::white);
    }

    int id = index.data(FriendModel::IdRole).toInt();
    QString nickname = index.data(FriendModel::NicknameRole).toString();
    bool isOnline = index.data(FriendModel::IsOnlineRole).toBool();
    int unread = index.data(FriendModel::UnreadCountRole).toInt();
    QString avatarUrl = index.data(FriendModel::AvatarUrlRole).toString();

    QRect avatarRect = option.rect.adjusted(10, 10, 0, -10);
    avatarRect.setWidth(40);
    avatarRect.setHeight(40);

    if (!avatarUrl.isEmpty() && avatarCache().contains(avatarUrl)) {
        QPixmap* cachedPixmap = avatarCache().object(avatarUrl);
        QPixmap scaled = cachedPixmap->scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainterPath path;
        path.addEllipse(avatarRect);
        painter->setClipPath(path);
        painter->drawPixmap(avatarRect, scaled);
        painter->setClipping(false);
    } else {
        painter->setBrush(AVATAR_BACKGROUND_COLOR);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(avatarRect);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
        painter->drawText(avatarRect, Qt::AlignCenter, nickname.left(1).toUpper());
        
        if (!avatarUrl.isEmpty()) {
            loadAvatar(avatarUrl);
        }
    }

    QRect nameRect = option.rect.adjusted(60, 10, -10, -30);
    painter->setPen(option.state & QStyle::State_Selected ? Qt::white : TEXT_COLOR_BLACK);
    painter->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nickname);

    QRect statusRect = option.rect.adjusted(60, 32, -10, -10);
    painter->setPen(isOnline ? ONLINE_STATUS_COLOR : OFFLINE_STATUS_COLOR);
    painter->setFont(QFont("Microsoft YaHei", 9));
    QString statusText = isOnline ? QString::fromUtf8("在线") : QString::fromUtf8("离线");
    painter->drawText(statusRect, Qt::AlignLeft | Qt::AlignVCenter, statusText);

    int statusDotSize = 8;
    int statusDotX = statusRect.left() - statusDotSize - 4;
    int statusDotY = statusRect.top() + (statusRect.height() - statusDotSize) / 2;
    QRect statusDotRect(statusDotX, statusDotY, statusDotSize, statusDotSize);
    painter->setBrush(isOnline ? ONLINE_STATUS_COLOR : OFFLINE_STATUS_COLOR);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(statusDotRect);

    if (unread > 0) {
        QString unreadText = unread > 99 ? "99+" : QString::number(unread);
        QFont unreadFont("Microsoft YaHei", 8, QFont::Bold);
        QFontMetrics fm(unreadFont);
        int textWidth = fm.horizontalAdvance(unreadText);
        int badgeWidth = qMax(textWidth + 10, 18);
        
        QRect badgeRect(option.rect.right() - badgeWidth - 10, option.rect.top() + 10, badgeWidth, 18);
        painter->setBrush(UNREAD_DOT_COLOR);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(badgeRect, 9, 9);
        
        painter->setPen(Qt::white);
        painter->setFont(unreadFont);
        painter->drawText(badgeRect, Qt::AlignCenter, unreadText);
    }

    painter->setPen(DIVIDER_COLOR);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    painter->restore();
}

QSize FriendDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
    return QSize(200, 60);
}
