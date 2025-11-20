#ifndef FRIENDMANAGEMENTDIALOG
#define FRIENDMANAGEMENTDIALOG

#include <QDialog>
#include "core/ApiService.h"
#include "ClickableLabel.h"
#include <QMouseEvent>
#include <QPoint>
namespace Ui {
class FriendManagementDialog;
}

class FriendManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FriendManagementDialog(ApiService* apiService,QWidget *parent = nullptr);
    ~FriendManagementDialog();

private slots:
    void on_esc_label_clicked();
    void on_max_label_clicked();
    void on_minus_label_clicked();

private:
    Ui::FriendManagementDialog *ui;
    ApiService* m_apiService; // 保存传入的指针
    // 新增：用于拖动窗口的变量
    bool m_isDragging;          // 标记是否正在拖动
    QPoint m_dragStartPosition; // 记录鼠标按下时的全局坐标
    QRect m_windowStartGeometry; // 记录窗口拖动前的几何位置

    // 新增：处理鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 新增：处理自定义顶部栏的鼠标事件
    void handleTopBarMousePress(QMouseEvent *event);
    void handleTopBarMouseMove(QMouseEvent *event);
    void handleTopBarMouseRelease(QMouseEvent *event);
};

#endif // FRIENDMANAGEMENTDIALOG
