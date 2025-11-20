// src/ui/ClickableLabel.cpp
#include "ClickableLabel.h"
#include <QMouseEvent>
#include <QApplication>

ClickableLabel::ClickableLabel(QWidget *parent)
    : QLabel(parent)
{
    setupAppearance();
}

ClickableLabel::ClickableLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setupAppearance();
}

void ClickableLabel::setupAppearance()
{
    setCursor(Qt::PointingHandCursor);
    setClickableStyle();
}

void ClickableLabel::setClickableStyle(bool underline, const QColor &color)
{
    QString style = QString("QLabel { color: %1; %2 } "
                            "QLabel:hover { color: %3; background-color: #f0f0f0; }")
                        .arg(color.name())
                        .arg(underline ? "text-decoration: underline;" : "")
                        .arg(color.darker(120).name());

    setStyleSheet(style);
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
        emit clickedWithPosition(event->pos());
    }
    QLabel::mousePressEvent(event);
}

void ClickableLabel::enterEvent(QEnterEvent *event)
{
    // 可以在这里添加悬停效果
    QLabel::enterEvent(event);
}

void ClickableLabel::leaveEvent(QEvent *event)
{
    // 可以在这里恢复原始样式
    QLabel::leaveEvent(event);
}
