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

static const QColor SELF_BUBBLE_COLOR(220, 248, 198);
static const QColor OTHER_BUBBLE_COLOR(241, 240, 240);
static const QColor BUBBLE_BORDER_COLOR(224, 224, 224);
static const QColor TEXT_COLOR(0, 0, 0);
static const QColor TIME_COLOR(136, 136, 136);
static const QColor FILE_BG_COLOR(240, 240, 240);
static const QColor FILE_ICON_COLOR(100, 100, 100);
static const QColor LOADING_COLOR(200, 200, 200);

QSize MessageDelegate::calculateTextSize(const QString &text) const {
    QTextDocument doc;
    doc.setHtml(text.toHtmlEscaped().replace("\n", "<br>"));
    doc.setTextWidth(280);
    return QSize(doc.idealWidth(), static_cast<int>(doc.size().height()));
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

    if (messageType == "image") {
        paintImageMessage(painter, option, isSelf, fileUrl, time);
    } else if (messageType == "file") {
        paintFileMessage(painter, option, isSelf, fileName, fileUrl, time);
    } else {
        paintTextMessage(painter, option, isSelf, text, time);
    }

    painter->restore();
}

void MessageDelegate::paintTextMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &text, const QString &time) const {
    QSize textSize = calculateTextSize(text);
    int bubbleWidth = qMin(textSize.width() + 20, 300);
    int bubbleHeight = textSize.height() + 20;

    int x = isSelf ? (option.rect.right() - bubbleWidth - 10) : 10;
    QRect bubbleRect(x, option.rect.top() + 5, bubbleWidth, bubbleHeight);

    QPainterPath path;
    path.addRoundedRect(bubbleRect, 12, 12);
    painter->setBrush(isSelf ? SELF_BUBBLE_COLOR : OTHER_BUBBLE_COLOR);
    painter->setPen(isSelf ? Qt::NoPen : QPen(BUBBLE_BORDER_COLOR, 1));
    painter->drawPath(path);

    QRect textRect = bubbleRect.adjusted(10, 10, -10, -10);
    painter->setPen(TEXT_COLOR);
    painter->setFont(QFont("Arial", 10));

    QTextDocument doc;
    doc.setHtml(text.toHtmlEscaped().replace("\n", "<br>"));
    doc.setTextWidth(textRect.width());
    painter->translate(textRect.topLeft());
    doc.drawContents(painter);
    painter->translate(-textRect.topLeft());

    painter->setFont(QFont("Arial", 8));
    painter->setPen(TIME_COLOR);
    int timeX = isSelf ? (bubbleRect.left()) : (bubbleRect.right() - 40);
    painter->drawText(QRect(timeX, bubbleRect.bottom() + 2, 40, 14), Qt::AlignHCenter, time);
}

void MessageDelegate::paintImageMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &fileUrl, const QString &time) const {
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
        painter->setFont(QFont("Arial", 10));
        painter->drawText(imageRect, Qt::AlignCenter, "加载中...");
        
        loadImage(fileUrl);
    }

    painter->setFont(QFont("Arial", 8));
    painter->setPen(TIME_COLOR);
    int timeX = isSelf ? (bubbleRect.left()) : (bubbleRect.right() - 40);
    painter->drawText(QRect(timeX, bubbleRect.bottom() + 2, 40, 14), Qt::AlignHCenter, time);
}

void MessageDelegate::paintFileMessage(QPainter *painter, const QStyleOptionViewItem &option, bool isSelf, const QString &fileName, const QString &fileUrl, const QString &time) const {
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
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(iconRect, Qt::AlignCenter, "FILE");

    QRect nameRect(x + 60, option.rect.top() + 20, bubbleWidth - 80, 30);
    painter->setPen(TEXT_COLOR);
    painter->setFont(QFont("Arial", 10));
    QString displayName = fileName.length() > 15 ? fileName.left(12) + "..." : fileName;
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);

    m_lastFileRect = bubbleRect;
    m_lastFileUrl = fileUrl;
    m_lastFileName = fileName;

    painter->setFont(QFont("Arial", 8));
    painter->setPen(TIME_COLOR);
    int timeX = isSelf ? (bubbleRect.left()) : (bubbleRect.right() - 40);
    painter->drawText(QRect(timeX, bubbleRect.bottom() + 2, 40, 14), Qt::AlignHCenter, time);
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
