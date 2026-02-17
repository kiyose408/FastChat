#ifndef FRIENDMANAGEMENTDIALOG
#define FRIENDMANAGEMENTDIALOG

#include <QDialog>
#include "core/ApiService.h"
#include "ClickableLabel.h"
#include "UserDelegate.h"
#include "FriendRequestDelegate.h"
#include <QMouseEvent>
#include <QPoint>
#include <QStandardItemModel>
namespace Ui {
class FriendManagementDialog;
}

class FriendManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FriendManagementDialog(ApiService* apiService,QWidget *parent = nullptr);
    ~FriendManagementDialog();
    
    void refreshFriendRequests();

private slots:
    void on_esc_label_clicked();
    void on_max_label_clicked();
    void on_minus_label_clicked();
    void on_search_btn_clicked();
    void onSearchUsersSuccess(const QJsonArray& users);
    void onSearchUsersFailed(const QString& error);
    void onSendFriendRequestSuccess(const QJsonObject& result);
    void onSendFriendRequestFailed(const QString& error);
    void onAddFriendRequested(int userId, const QString& note);
    
    void onGetFriendRequestsSuccess(const QJsonArray& requests);
    void onGetFriendRequestsFailed(const QString& error);
    void onAcceptFriendRequest(int userId);
    void onRejectFriendRequest(int userId);
    void onAcceptFriendRequestSuccess(const QJsonObject& result);
    void onAcceptFriendRequestFailed(const QString& error);
    void onRejectFriendRequestSuccess();
    void onRejectFriendRequestFailed(const QString& error);

private:
    Ui::FriendManagementDialog *ui;
    ApiService* m_apiService;
    bool m_isDragging;
    QPoint m_dragStartPosition;
    QRect m_windowStartGeometry;
    QStandardItemModel* m_searchModel;
    UserDelegate* m_searchDelegate;
    QStandardItemModel* m_requestModel;
    FriendRequestDelegate* m_requestDelegate;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void handleTopBarMousePress(QMouseEvent *event);
    void handleTopBarMouseMove(QMouseEvent *event);
    void handleTopBarMouseRelease(QMouseEvent *event);
};

#endif // FRIENDMANAGEMENTDIALOG
