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
    /**
     * @brief 构造函数
     * 初始化网络管理器并连接其 finished 信号到 onReplyFinished 槽函数。
     * @param parent 可选的父 QObject，用于自动内存管理
     */
    explicit ApiService(QObject *parent = nullptr);

    void registerUser(const QString &username, const QString &email, const QString &password);
    /**
     * @brief 发起用户登录请求
     * 将用户名和密码以 JSON 格式 POST 到 /api/auth/login 接口
     * @param username 用户名
     * @param password 密码
     */
    void login(const QString& username, const QString& password);

    /**
     * @brief 获取当前用户的个人信息
     * 需要已登录并持有有效 token 才能发起请求
     */
    void fetchUserInfo();

    /**
     * @brief 获取当前用户的好友列表
     * 需要已登录并持有有效 token 才能发起请求
     */
    void fetchFriends();

signals:
    // 新增的注册相关信号
    void registerSuccess(const QJsonObject &user, const QString &token);
    void registerFailed(const QString &error);
    /**
     * @brief 登录成功信号
     * 当服务器返回有效 token 和用户数据时触发
     * @param user 包含用户信息的 JSON 对象（如 id, username）
     * @param token JWT 认证令牌
     */
    void loginSuccess(const QJsonObject& user, const QString& token);

    /**
     * @brief 登录失败信号
     * 当网络错误或认证失败时触发
     * @param error 错误描述字符串
     */
    void loginFailed(const QString& error);

    /**
     * @brief 用户信息获取成功信号
     * 当成功从服务器获取用户资料时触发
     * @param user 用户信息 JSON 对象
     */
    void userInfoFetched(const QJsonObject& user);

    /**
     * @brief 好友列表获取成功信号
     * 当成功获取好友列表时触发
     * @param friends 好友数组（每个元素为一个用户对象）
     */
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
