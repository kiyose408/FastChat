#include "FriendManagementDialog.h"
#include "ui_FriendManagementDialog.h"
#include <QStandardItemModel>
#include <QStandardItem>

FriendManagementDialog::FriendManagementDialog(ApiService* apiService,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FriendManagementDialog)
    , m_apiService(apiService)
    ,m_isDragging(false)
    , m_searchModel(new QStandardItemModel(this))
    , m_searchDelegate(new UserDelegate(this))
    , m_requestModel(new QStandardItemModel(this))
    , m_requestDelegate(new FriendRequestDelegate(this))
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowTitle("FastChat-好友管理");
    
    ui->searchListView->setModel(m_searchModel);
    ui->searchListView->setItemDelegate(m_searchDelegate);
    ui->searchListView->setMouseTracking(true);
    
    ui->requestListView->setModel(m_requestModel);
    ui->requestListView->setItemDelegate(m_requestDelegate);
    ui->requestListView->setMouseTracking(true);
    
    connect(m_apiService, &ApiService::searchUsersSuccess, this, &FriendManagementDialog::onSearchUsersSuccess);
    connect(m_apiService, &ApiService::searchUsersFailed, this, &FriendManagementDialog::onSearchUsersFailed);
    connect(m_apiService, &ApiService::sendFriendRequestSuccess, this, &FriendManagementDialog::onSendFriendRequestSuccess);
    connect(m_apiService, &ApiService::sendFriendRequestFailed, this, &FriendManagementDialog::onSendFriendRequestFailed);
    
    connect(m_apiService, &ApiService::getFriendRequestsSuccess, this, &FriendManagementDialog::onGetFriendRequestsSuccess);
    connect(m_apiService, &ApiService::getFriendRequestsFailed, this, &FriendManagementDialog::onGetFriendRequestsFailed);
    connect(m_apiService, &ApiService::acceptFriendRequestSuccess, this, &FriendManagementDialog::onAcceptFriendRequestSuccess);
    connect(m_apiService, &ApiService::acceptFriendRequestFailed, this, &FriendManagementDialog::onAcceptFriendRequestFailed);
    connect(m_apiService, &ApiService::rejectFriendRequestSuccess, this, &FriendManagementDialog::onRejectFriendRequestSuccess);
    connect(m_apiService, &ApiService::rejectFriendRequestFailed, this, &FriendManagementDialog::onRejectFriendRequestFailed);
    
    connect(m_searchDelegate, &UserDelegate::addFriendRequested, this, &FriendManagementDialog::onAddFriendRequested);
    
    connect(m_requestDelegate, &FriendRequestDelegate::acceptRequest, this, &FriendManagementDialog::onAcceptFriendRequest);
    connect(m_requestDelegate, &FriendRequestDelegate::rejectRequest, this, &FriendManagementDialog::onRejectFriendRequest);
    
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &FriendManagementDialog::on_search_btn_clicked);
    
    refreshFriendRequests();
}

FriendManagementDialog::~FriendManagementDialog()
{
    delete ui;
}

void FriendManagementDialog::refreshFriendRequests()
{
    m_apiService->getFriendRequests();
}

void FriendManagementDialog::on_esc_label_clicked()
{
    close();
}


void FriendManagementDialog::on_max_label_clicked()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}


void FriendManagementDialog::on_minus_label_clicked()
{
    FriendManagementDialog::showMinimized();
}

void FriendManagementDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPosition().toPoint();
        m_windowStartGeometry = geometry();
    }
    QWidget::mousePressEvent(event);
}

void FriendManagementDialog::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging && event->buttons() & Qt::LeftButton) {
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPosition;
        move(m_windowStartGeometry.topLeft() + delta);
    }
    QWidget::mouseMoveEvent(event);
}

void FriendManagementDialog::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void FriendManagementDialog::on_search_btn_clicked()
{
    QString query = ui->lineEdit->text().trimmed();
    if (query.isEmpty()) {
        return;
    }
    
    m_apiService->searchUsers(query);
}

void FriendManagementDialog::onSearchUsersSuccess(const QJsonArray& users)
{
    qDebug() << "搜索成功，找到" << users.size() << "个用户";
    
    m_searchModel->clear();
    
    for (const QJsonValue& user : users) {
        QJsonObject userObj = user.toObject();
        int userId = userObj["id"].toInt();
        QString username = userObj["username"].toString();
        QString email = userObj["email"].toString();
        
        qDebug() << "用户ID:" << userId << "用户名:" << username << "邮箱:" << email;
        
        QStandardItem* item = new QStandardItem(username);
        item->setData(userId, Qt::UserRole);
        item->setData(email, Qt::UserRole + 1);
        m_searchModel->appendRow(item);
    }
}

void FriendManagementDialog::onSearchUsersFailed(const QString& error)
{
    qDebug() << "搜索失败:" << error;
}

void FriendManagementDialog::onSendFriendRequestSuccess(const QJsonObject& result)
{
    qDebug() << "发送好友请求成功:" << result["message"].toString();
}

void FriendManagementDialog::onSendFriendRequestFailed(const QString& error)
{
    qDebug() << "发送好友请求失败:" << error;
}

void FriendManagementDialog::onAddFriendRequested(int userId, const QString& note)
{
    qDebug() << "请求添加好友，用户ID:" << userId << "备注:" << note;
    
    m_apiService->sendFriendRequest(userId, note);
}

void FriendManagementDialog::onGetFriendRequestsSuccess(const QJsonArray& requests)
{
    qDebug() << "获取好友请求成功，共" << requests.size() << "个请求";
    
    m_requestModel->clear();
    
    for (const QJsonValue& request : requests) {
        QJsonObject requestObj = request.toObject();
        int userId = requestObj["user_id"].toInt();
        QString username = requestObj["username"].toString();
        QString email = requestObj["email"].toString();
        QString note = requestObj["note"].toString();
        
        qDebug() << "好友请求 - 用户ID:" << userId << "用户名:" << username << "邮箱:" << email << "备注:" << note;
        
        QStandardItem* item = new QStandardItem(username);
        item->setData(userId, Qt::UserRole);
        item->setData(email, Qt::UserRole + 1);
        item->setData(note, Qt::UserRole + 2);
        m_requestModel->appendRow(item);
    }
}

void FriendManagementDialog::onGetFriendRequestsFailed(const QString& error)
{
    qDebug() << "获取好友请求失败:" << error;
}

void FriendManagementDialog::onAcceptFriendRequest(int userId)
{
    qDebug() << "接受好友请求，用户ID:" << userId;
    m_apiService->acceptFriendRequest(userId);
}

void FriendManagementDialog::onRejectFriendRequest(int userId)
{
    qDebug() << "拒绝好友请求，用户ID:" << userId;
    m_apiService->rejectFriendRequest(userId);
}

void FriendManagementDialog::onAcceptFriendRequestSuccess(const QJsonObject& result)
{
    qDebug() << "接受好友请求成功:" << result["message"].toString();
    refreshFriendRequests();
}

void FriendManagementDialog::onAcceptFriendRequestFailed(const QString& error)
{
    qDebug() << "接受好友请求失败:" << error;
}

void FriendManagementDialog::onRejectFriendRequestSuccess()
{
    qDebug() << "拒绝好友请求成功";
    refreshFriendRequests();
}

void FriendManagementDialog::onRejectFriendRequestFailed(const QString& error)
{
    qDebug() << "拒绝好友请求失败:" << error;
}
