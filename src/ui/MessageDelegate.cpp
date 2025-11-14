#include "MessageDelegate.h"
#include "core/MessageModel.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QTextDocument>

// =============== 颜色主题配置区（便于后期维护和换肤） ===============
static const QColor SELF_BUBBLE_COLOR(220, 248, 198);   // #DCF8C6 - 自己的消息气泡
static const QColor OTHER_BUBBLE_COLOR(241, 240, 240);  // #F1F0F0 - 对方的消息气泡
static const QColor BUBBLE_BORDER_COLOR(224, 224, 224); // #E0E0E0 - 气泡边框
static const QColor TEXT_COLOR(0, 0, 0);                // 黑色文字
static const QColor TIME_COLOR(136, 136, 136);          // #888888 - 时间文字
// =======================================================

QSize MessageDelegate::calculateTextSize(const QString &text) const {
    QTextDocument doc;
    doc.setHtml(text.toHtmlEscaped().replace("\n", "<br>"));
    doc.setTextWidth(280); // 最大宽度
    return QSize(doc.idealWidth(), static_cast<int>(doc.size().height()));
}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    bool isSelf = index.data(MessageModel::IsSelfRole).toBool();
    QString text = index.data(MessageModel::TextRole).toString();
    QString time = index.data(MessageModel::TimeRole).toString();

    // 计算文本尺寸
    QSize textSize = calculateTextSize(text);
    int bubbleWidth = qMin(textSize.width() + 20, 300);
    int bubbleHeight = textSize.height() + 20;

    // 气泡位置
    int x = isSelf ? (option.rect.right() - bubbleWidth - 10) : 10;
    QRect bubbleRect(x, option.rect.top() + 5, bubbleWidth, bubbleHeight);

    // 绘制气泡背景
    QPainterPath path;
    path.addRoundedRect(bubbleRect, 12, 12);
    painter->setBrush(isSelf ? SELF_BUBBLE_COLOR : OTHER_BUBBLE_COLOR);
    painter->setPen(isSelf ? Qt::NoPen : QPen(BUBBLE_BORDER_COLOR, 1));
    painter->drawPath(path);

    // 绘制文本
    QRect textRect = bubbleRect.adjusted(10, 10, -10, -10);
    painter->setPen(TEXT_COLOR);
    painter->setFont(QFont("Arial", 10));

    QTextDocument doc;
    doc.setHtml(text.toHtmlEscaped().replace("\n", "<br>"));
    doc.setTextWidth(textRect.width());
    painter->translate(textRect.topLeft());
    doc.drawContents(painter);
    painter->translate(-textRect.topLeft());

    // 绘制时间
    painter->setFont(QFont("Arial", 8));
    painter->setPen(TIME_COLOR);
    int timeX = isSelf ? (bubbleRect.left()) : (bubbleRect.right() - 40);
    painter->drawText(QRect(timeX, bubbleRect.bottom() + 2, 40, 14), Qt::AlignHCenter, time);

    painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QString text = index.data(MessageModel::TextRole).toString();
    QSize textSize = calculateTextSize(text);
    return QSize(option.rect.width(), textSize.height() + 40);
}
