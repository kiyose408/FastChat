#ifndef CHATMAINWINDOW_H
#define CHATMAINWINDOW_H

#include <QMainWindow>
#include "core/configmanager.h"
#include "core/FriendModel.h"
#include "ui/FriendDelegate.h"
#include "core/MessageModel.h"
#include "ui/MessageDelegate.h"
#include "core/WebSocketClient.h"
#include "core/ApiService.h"
#include "core/SessionManager.h"
#include "ui/ClickableLabel.h"
#include "ui/FriendManagementDialog.h"
class FriendModel;
class MessageModel;
QT_BEGIN_NAMESPACE
namespace Ui {
class ChatMainWindow;
}
QT_END_NAMESPACE

class ChatMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ChatMainWindow(QWidget *parent = nullptr);
    void toggleMaxmize();
    ~ChatMainWindow();
private slots:
    void onFriendClicked(const QModelIndex &index);
    void on_addFriend_label_clicked();

    void on_esc_label_clicked();
    void on_max_label_clicked();
    void on_minus_label_clicked();

private:
    Ui::ChatMainWindow *ui;
    ApiService* m_apiService;
    FriendModel *m_friendModel;
    WebSocketClient* m_webSocketClient;
    SessionManager& m_sessionManager;
    MessageModel *m_messageModel;
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
#endif // CHATMAINWINDOW_H
