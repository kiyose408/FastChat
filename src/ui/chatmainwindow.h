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
#include "ui/FriendInfoDialog.h"
#include <QLabel>
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
    
    void updateFriendRequestBadge(int count);
    void refreshFriendList();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onFriendClicked(const QModelIndex &index);
    void on_addFriend_label_clicked();
    void on_more_label_clicked();
    void on_pic_label_clicked();
    void on_file_label_clicked();

    void on_esc_label_clicked();
    void on_max_label_clicked();
    void on_minus_label_clicked();
    
    void onGetFriendRequestsSuccess(const QJsonArray& requests);
    void onGetFriendRequestsFailed(const QString& error);
    
    void onGetFriendsSuccess(const QJsonArray& friends);
    void onGetFriendsFailed(const QString& error);
    
    void on_send_btn_clicked();
    void onSendMessageSuccess(const QJsonObject& message);
    void onSendMessageFailed(const QString& error);
    void onGetConversationSuccess(const QJsonArray& messages);
    void onGetConversationFailed(const QString& error);
    
    void onDeleteFriendSuccess();
    
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketError(const QString& error);
    void onWebSocketMessageReceived(const QJsonObject& message);
    void onWebSocketMessageSent(const QJsonObject& message);
    void onWebSocketFriendRequestReceived(int userId, const QString& username, const QString& note);
    void onWebSocketFriendRequestAccepted(int friendId, const QString& username);
    void onWebSocketFriendDeleted(int friendId);
    void onWebSocketUserStatusChanged(int userId, bool isOnline);
    void onWebSocketMessagesRead(int recipientId);
    void onWebSocketMessageRecalled(int messageId);
    
    void onUploadImageSuccess(const QJsonObject& result);
    void onUploadImageFailed(const QString& error);
    void onUploadFileSuccess(const QJsonObject& result);
    void onUploadFileFailed(const QString& error);

private:
    Ui::ChatMainWindow *ui;
    ApiService* m_apiService;
    FriendModel *m_friendModel;
    WebSocketClient* m_webSocketClient;
    SessionManager& m_sessionManager;
    MessageModel *m_messageModel;
    bool m_isDragging;
    QPoint m_dragStartPosition;
    QRect m_windowStartGeometry;
    QLabel* m_friendRequestBadge;
    int m_friendRequestCount;
    int m_currentFriendId;
    QString m_currentFriendName;
    bool m_hasUnreadMessages;
    int m_selectedMessageId;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void handleTopBarMousePress(QMouseEvent *event);
    void handleTopBarMouseMove(QMouseEvent *event);
    void handleTopBarMouseRelease(QMouseEvent *event);
    
    void markCurrentChatAsRead();
    void showContextMenu(const QPoint &pos);
    void onRecallMessage();
};
#endif // CHATMAINWINDOW_H
