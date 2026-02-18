#include "MessageDelegate.h"
#include "core/MessageModel.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QTextDocument>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QDebug>
#include <QTextLayout>
#include <QTextOption>

static const QColor SELF_BUBBLE_COLOR(220, 248, 198);
static const QColor OTHER_BUBBLE_COLOR(241, 240, 240);
static const QColor BUBBLE_BORDER_COLOR(224, 224, 224);
static const QColor TEXT_COLOR(0, 0, 0);
static const QColor TIME_COLOR(136, 136, 136);
static const QColor FILE_BG_COLOR(240, 240, 240);
static const QColor FILE_ICON_COLOR(100, 100, 100);
static const QColor LOADING_COLOR(200, 200, 200);
static const QColor READ_COLOR(136, 136, 136);
static const QColor UNREAD_COLOR(100, 149, 237);
static const QColor RECALLED_COLOR(150, 150, 150);

QSize MessageDelegate::calculateTextSize(const QString &text) const {
    QFont font("Microsoft YaHei", 10);
    QFontMetrics fm(font);
    
    int maxWidth = 280;
    
    QStringList lines;
    QString currentLine;
    int currentLineWidth = 0;
    
    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i];
        int charWidth = fm.horizontalAdvance(c);
        
        if (c == '\n') {
            lines.append(currentLine);
            currentLine.clear();
            currentLineWidth = 0;
        } else if (currentLineWidth + charWidth > maxWidth) {
            lines.append(currentLine);
            currentLine = c;
            currentLineWidth = charWidth;
        } else {
            currentLine += c;
            currentLineWidth += charWidth;
        }
    }
    if (!currentLine.isEmpty()) {
        lines.append(currentLine);
    }
    
    int totalHeight = lines.count() * fm.height();
    
    int maxLineWidth = 0;
    for (const QString& line : lines) {
        int lineWidth = fm.horizontalAdvance(line);
        if (lineWidth > maxLineWidth) {
            maxLineWidth = lineWidth;
        }
    }
    
    return QSize(maxLineWidth, totalHeight);
}

void MessageDelegate::loadImage(const QString& url) const {
    if (m_loadingImages.contains(url) || m_imageCache.contains(url)) {
        return;
    }
    
    m_loadingImages[url] = true;
    QString fullUrl = "http://localhost:3000" + url;
    
    QUrl qurl(fullUrl);
    QNetworkRequest request(qurl);
    QNetworkReply* reply = m_networkManager.get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        m_loadingImages.remove(url);
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap* pixmap = new QPixmap();
            if (pixmap->loadFromData(data)) {
                m_imageCache.insert(url, pixmap);
            } else {
                delete pixmap;
            }
        }
        reply->deleteLater();
    });
}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    bool isSelf = index.data(MessageModel::IsSelfRole).toBool();
    QString text = index.data(MessageModel::TextRole).toString();
    QString time = index.data(MessageModel::TimeRole).toString();
    QString messageType = index.data(MessageModel::MessageTypeRole).toString();
    QString fileUrl = index.data(MessageModel::FileUrlRole).toString();
    QString fileName = index.data(MessageModel::FileNameRole).toString();
    bool isRead = index.data(MessageModel::IsReadRole).toBool();
    bool isRecalled = index.data(MessageModel::IsRecalledRole).toBool();

    if (isRecalled) {
        paintRecalledMessage(painter, option, isSelf);
    } else if (messageType == "image") {
        paintImageMessage(painter, option, isSelf, fileUrl, time, isRead);
    } else if (messageType == "file") {
        paintFileMessage(painter, option, isSelf, fileName, fileUrl, time, isRead);
    } else {
        paintTextMessage(painter, option, isSelf, text, time, isRead);
    }

    painter->restore();
}

void MessageDelegate::paintTextMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &text, const QString &time, bool isRead) const {
    QFont font("Microsoft YaHei", 10);
    QFontMetrics fm(font);
    
    int maxWidth = 280;
    int padding = 10;
    
    QStringList lines;
    QString currentLine;
    int currentLineWidth = 0;
    
    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i];
        int charWidth = fm.horizontalAdvance(c);
        
        if (c == '\n') {
            lines.append(currentLine);
            currentLine.clear();
            currentLineWidth = 0;
        } else if (currentLineWidth + charWidth > maxWidth) {
            lines.append(currentLine);
            currentLine = c;
            currentLineWidth = charWidth;
        } else {
            currentLine += c;
            currentLineWidth += charWidth;
        }
    }
    if (!currentLine.isEmpty()) {
        lines.append(currentLine);
    }
    
    int totalHeight = lines.count() * fm.height();
    
    int maxLineWidth = 0;
    for (const QString& line : lines) {
        int lineWidth = fm.horizontalAdvance(line);
        if (lineWidth > maxLineWidth) {
            maxLineWidth = lineWidth;
        }
    }
    
    int bubbleWidth = qMin(maxLineWidth + padding * 2, maxWidth + padding * 2);
    int bubbleHeight = totalHeight + padding * 2;

    int x = isSelf ? (option.rect.right() - bubbleWidth - 10) : 10;
    QRect bubbleRect(x, option.rect.top() + 5, bubbleWidth, bubbleHeight);

    QPainterPath path;
    path.addRoundedRect(bubbleRect, 12, 12);
    painter->setBrush(isSelf ? SELF_BUBBLE_COLOR : OTHER_BUBBLE_COLOR);
    painter->setPen(isSelf ? Qt::NoPen : QPen(BUBBLE_BORDER_COLOR, 1));
    painter->drawPath(path);

    painter->setFont(font);
    painter->setPen(TEXT_COLOR);
    
    int textY = bubbleRect.top() + padding + fm.ascent();
    for (const QString& line : lines) {
        painter->drawText(bubbleRect.left() + padding, textY, line);
        textY += fm.height();
    }

    QFont timeFont("Microsoft YaHei", 8);
    painter->setFont(timeFont);
    painter->setPen(TIME_COLOR);
    
    if (isSelf) {
        QString readText = isRead ? QString::fromUtf8("已读") : QString::fromUtf8("未读");
        painter->setPen(isRead ? READ_COLOR : UNREAD_COLOR);
        painter->drawText(QRect(bubbleRect.left(), bubbleRect.bottom() + 2, 30, 14), Qt::AlignLeft, readText);
        painter->setPen(TIME_COLOR);
        painter->drawText(QRect(bubbleRect.left() + 32, bubbleRect.bottom() + 2, 40, 14), Qt::AlignLeft, time);
    } else {
        painter->drawText(QRect(bubbleRect.right() - 40, bubbleRect.bottom() + 2, 40, 14), Qt::AlignRight, time);
    }
}

void MessageDelegate::paintImageMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &fileUrl, const QString &time, bool isRead) const {
    int maxImageWidth = 200;
    int maxImageHeight = 150;
    
    int bubbleWidth = maxImageWidth + 20;
    int bubbleHeight = maxImageHeight + 20;

    int x = isSelf ? (option.rect.right() - bubbleWidth - 10) : 10;
    QRect bubbleRect(x, option.rect.top() + 5, bubbleWidth, bubbleHeight);

    QPainterPath path;
    path.addRoundedRect(bubbleRect, 12, 12);
    painter->setBrush(isSelf ? SELF_BUBBLE_COLOR : OTHER_BUBBLE_COLOR);
    painter->setPen(isSelf ? Qt::NoPen : QPen(BUBBLE_BORDER_COLOR, 1));
    painter->drawPath(path);

    QRect imageRect(x + 10, option.rect.top() + 10, maxImageWidth, maxImageHeight);
    
    m_lastImageRect = imageRect;
    m_lastImageFileUrl = fileUrl;
    
    if (m_imageCache.contains(fileUrl)) {
        QPixmap* cachedPixmap = m_imageCache.object(fileUrl);
        QPixmap scaled = cachedPixmap->scaled(maxImageWidth, maxImageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        int drawX = imageRect.left() + (maxImageWidth - scaled.width()) / 2;
        int drawY = imageRect.top() + (maxImageHeight - scaled.height()) / 2;
        
        painter->drawPixmap(drawX, drawY, scaled);
    } else {
        painter->setBrush(LOADING_COLOR);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(imageRect, 8, 8);
        
        painter->setPen(Qt::white);
        painter->setFont(QFont("Microsoft YaHei", 10));
        painter->drawText(imageRect, Qt::AlignCenter, QString::fromUtf8("加载中..."));
        
        loadImage(fileUrl);
    }

    QFont timeFont("Microsoft YaHei", 8);
    painter->setFont(timeFont);
    painter->setPen(TIME_COLOR);
    
    if (isSelf) {
        QString readText = isRead ? QString::fromUtf8("已读") : QString::fromUtf8("未读");
        painter->setPen(isRead ? READ_COLOR : UNREAD_COLOR);
        painter->drawText(QRect(bubbleRect.left(), bubbleRect.bottom() + 2, 30, 14), Qt::AlignLeft, readText);
        painter->setPen(TIME_COLOR);
        painter->drawText(QRect(bubbleRect.left() + 32, bubbleRect.bottom() + 2, 40, 14), Qt::AlignLeft, time);
    } else {
        painter->drawText(QRect(bubbleRect.right() - 40, bubbleRect.bottom() + 2, 40, 14), Qt::AlignRight, time);
    }
}

void MessageDelegate::paintFileMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &fileName, const QString &fileUrl, const QString &time, bool isRead) const {
    int bubbleWidth = 220;
    int bubbleHeight = 60;

    int x = isSelf ? (option.rect.right() - bubbleWidth - 10) : 10;
    QRect bubbleRect(x, option.rect.top() + 5, bubbleWidth, bubbleHeight);

    QPainterPath path;
    path.addRoundedRect(bubbleRect, 12, 12);
    painter->setBrush(isSelf ? SELF_BUBBLE_COLOR : OTHER_BUBBLE_COLOR);
    painter->setPen(isSelf ? Qt::NoPen : QPen(BUBBLE_BORDER_COLOR, 1));
    painter->drawPath(path);

    QRect iconRect(x + 10, option.rect.top() + 15, 40, 40);
    painter->setBrush(FILE_BG_COLOR);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(iconRect, 8, 8);
    
    painter->setPen(FILE_ICON_COLOR);
    painter->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    painter->drawText(iconRect, Qt::AlignCenter, "FILE");

    QRect nameRect(x + 60, option.rect.top() + 20, bubbleWidth - 80, 30);
    painter->setPen(TEXT_COLOR);
    painter->setFont(QFont("Microsoft YaHei", 10));
    QString displayName = fileName.length() > 15 ? fileName.left(12) + "..." : fileName;
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);

    m_lastFileRect = bubbleRect;
    m_lastFileUrl = fileUrl;
    m_lastFileName = fileName;

    QFont timeFont("Microsoft YaHei", 8);
    painter->setFont(timeFont);
    painter->setPen(TIME_COLOR);
    
    if (isSelf) {
        QString readText = isRead ? QString::fromUtf8("已读") : QString::fromUtf8("未读");
        painter->setPen(isRead ? READ_COLOR : UNREAD_COLOR);
        painter->drawText(QRect(bubbleRect.left(), bubbleRect.bottom() + 2, 30, 14), Qt::AlignLeft, readText);
        painter->setPen(TIME_COLOR);
        painter->drawText(QRect(bubbleRect.left() + 32, bubbleRect.bottom() + 2, 40, 14), Qt::AlignLeft, time);
    } else {
        painter->drawText(QRect(bubbleRect.right() - 40, bubbleRect.bottom() + 2, 40, 14), Qt::AlignRight, time);
    }
}

void MessageDelegate::paintRecalledMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf) const {
    QFont font("Microsoft YaHei", 10);
    QFontMetrics fm(font);
    
    QString recalledText = isSelf ? QString::fromUtf8("你撤回了一条消息") : QString::fromUtf8("对方撤回了一条消息");
    int textWidth = fm.horizontalAdvance(recalledText);
    
    int bubbleWidth = textWidth + 20;
    int bubbleHeight = 30;

    int x = isSelf ? (option.rect.right() - bubbleWidth - 10) : 10;
    QRect bubbleRect(x, option.rect.top() + 5, bubbleWidth, bubbleHeight);

    painter->setBrush(OTHER_BUBBLE_COLOR);
    painter->setPen(QPen(BUBBLE_BORDER_COLOR, 1));
    QPainterPath path;
    path.addRoundedRect(bubbleRect, 12, 12);
    painter->drawPath(path);

    painter->setFont(font);
    painter->setPen(RECALLED_COLOR);
    painter->drawText(bubbleRect, Qt::AlignCenter, recalledText);
}

bool MessageDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    Q_UNUSED(model);
    
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QString messageType = index.data(MessageModel::MessageTypeRole).toString();
        QString fileUrl = index.data(MessageModel::FileUrlRole).toString();
        QString fileName = index.data(MessageModel::FileNameRole).toString();
        
        if (messageType == "image" && m_lastImageRect.contains(mouseEvent->pos())) {
            QString fullUrl = "http://localhost:3000" + fileUrl;
            QDesktopServices::openUrl(QUrl(fullUrl));
            return true;
        } else if (messageType == "file" && m_lastFileRect.contains(mouseEvent->pos())) {
            QString fullUrl = "http://localhost:3000" + fileUrl;
            QDesktopServices::openUrl(QUrl(fullUrl));
            return true;
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    bool isRecalled = index.data(MessageModel::IsRecalledRole).toBool();
    
    if (isRecalled) {
        return QSize(option.rect.width(), 40);
    }
    
    QString messageType = index.data(MessageModel::MessageTypeRole).toString();
    
    if (messageType == "image") {
        return QSize(option.rect.width(), 200);
    } else if (messageType == "file") {
        return QSize(option.rect.width(), 90);
    } else {
        QString text = index.data(MessageModel::TextRole).toString();
        QSize textSize = calculateTextSize(text);
        return QSize(option.rect.width(), textSize.height() + 40);
    }
}
