// src/ui/ClickableLabel.h
#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr);

    // 可选：添加样式设置方法
    void setClickableStyle(bool underline = true, const QColor &color = Qt::blue);

signals:
    void clicked(); // 点击信号
    void clickedWithPosition(const QPoint& pos); // 带坐标的点击信号

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override; // 鼠标进入
    void leaveEvent(QEvent *event) override;      // 鼠标离开

private:
    void setupAppearance();
};

#endif // CLICKABLELABEL_H
