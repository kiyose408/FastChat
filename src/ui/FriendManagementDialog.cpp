#include "FriendManagementDialog.h"
#include "ui_FriendManagementDialog.h"

FriendManagementDialog::FriendManagementDialog(ApiService* apiService,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FriendManagementDialog)
    , m_apiService(apiService)
    ,m_isDragging(false)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint); // 取消注释这行来隐藏关闭按钮
    setWindowFlags(Qt::FramelessWindowHint); // 取消注释这行来完全移除标题栏和边框
    setWindowTitle("FastChat-好友管理"); // 或者留空 setWindowTitle("");
}

FriendManagementDialog::~FriendManagementDialog()
{
    delete ui;
}

void FriendManagementDialog::on_esc_label_clicked()
{
    close();
}


void FriendManagementDialog::on_max_label_clicked()
{
    if (isMaximized()) {
        showNormal(); // 还原
    } else {
        showMaximized(); // 最大化
    }
}


void FriendManagementDialog::on_minus_label_clicked()
{
    FriendManagementDialog::showMinimized();
}
// --- 重写的鼠标事件处理函数 ---
void FriendManagementDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPosition().toPoint();
        m_windowStartGeometry = geometry(); // 记录窗口开始拖动时的位置和大小
        // qDebug() << "Mouse Pressed at global pos:" << m_dragStartPosition;
    }
    QWidget::mousePressEvent(event); // 调用基类处理
}

void FriendManagementDialog::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging && event->buttons() & Qt::LeftButton) {
        // 计算鼠标移动的偏移量
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPosition;
        // 更新窗口位置
        move(m_windowStartGeometry.topLeft() + delta);
        // qDebug() << "Moving window to:" << m_windowStartGeometry.topLeft() + delta;
    }
    QWidget::mouseMoveEvent(event); // 调用基类处理
}

void FriendManagementDialog::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        // qDebug() << "Mouse Released";
    }
    QWidget::mouseReleaseEvent(event); // 调用基类处理
}

