#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief ApiService 类
 * 提供与后端 RESTful API 进行通信的功能，如登录、获取用户信息、获取好友列表等。
 * 所有网络请求均通过 QNetworkAccessManager 异步执行，并在完成时通过信号通知调用方。
 *
 * 使用示例：
 *   ApiService api;
 *   connect(&api, &ApiService::loginSuccess, [](const QJsonObject& user, const QString& token) {
 *       qDebug() << "Login successful:" << user["username"].toString();
 *   });
 *   api.login("alice", "password123");
 */
class ApiService : public QObject
{
    Q_OBJECT  // 启用 Qt 的元对象系统（用于信号/槽机制）

public:

    explicit ApiService(QObject *parent = nullptr);

    //注册和登录
    void registerUser(const QString &username, const QString &email, const QString &password);
    void login(const QString& username, const QString& password);

    //新增：好友相关API
    void searchUsers(const QString& query);
    void sendFriendRequest(int friendId);
    void getFriends();
    void getFriendRequests();
    void acceptFriendRequest(int requesterId);
    void rejectFriendRequest(int requesterId);

    void fetchUserInfo();
    void fetchFriends();

signals:

    void registerSuccess(const QJsonObject &user, const QString &token);
    void registerFailed(const QString &error);

    void loginSuccess(const QJsonObject& user, const QString& token);
    void loginFailed(const QString& error);


    // 新增：好友相关信号
    void searchUsersSuccess(const QJsonArray& users);
    void searchUsersFailed(const QString& error);
    void sendFriendRequestSuccess(const QJsonObject& request);
    void sendFriendRequestFailed(const QString& error);
    void getFriendsSuccess(const QJsonArray& friends);
    void getFriendsFailed(const QString& error);
    void getFriendRequestsSuccess(const QJsonArray& requests);
    void getFriendRequestsFailed(const QString& error);
    void acceptFriendRequestSuccess(const QJsonObject& request);
    void acceptFriendRequestFailed(const QString& error);
    void rejectFriendRequestSuccess();
    void rejectFriendRequestFailed(const QString& error);

    void userInfoFetched(const QJsonObject& user);
    void friendsFetched(const QJsonArray& friends);

private slots:
    /**
     * @brief 处理网络请求完成后的响应
     * 解析服务器返回的数据，并根据请求路径分发处理结果
     * @param reply 网络应答对象指针
     */
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_netManager;  // 网络访问管理器，负责发送 HTTP 请求

    /**
     * @brief 获取基础 URL
     * 所有 API 请求都基于此地址拼接路径
     * @return 返回后端服务的基础地址（例如 http://localhost:3000）
     */
    QString baseUrl() const { return "http://localhost:3000"; }
    //QString baseUrl() const { return "http://47.121.131.251:3000"; }
};

#endif // APISERVICE_H
