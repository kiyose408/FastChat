// ApiService.cpp
// 实现了 ApiService 类，该类负责与后端 API 进行网络通信。
// 它封装了登录、获取用户信息和获取好友列表等 API 调用逻辑，
// 并通过 Qt 的 QNetworkAccessManager 处理 HTTP 请求和响应。
// 该类依赖于 SessionManager 来管理认证 token。

#include "ApiService.h" // 包含头文件声明
#include "SessionManager.h"
#include <QNetworkRequest> // 用于构建 HTTP 请求
#include <QJsonDocument>   // 用于解析和构建 JSON 数据
#include <QJsonObject>     // 用于处理 JSON 对象
#include <QUrlQuery>       // 用于处理 URL 查询参数（虽然此文件未使用，但包含在内）
#include <QDebug>          // 用于调试输出


ApiService::ApiService(QObject *parent)
    : QObject(parent) // 调用父类 QObject 的构造函数
{
    // 连接 QNetworkAccessManager 的 finished 信号到当前对象的 onReplyFinished 槽函数
    // 当任何网络请求完成后，都会触发此信号，从而调用 onReplyFinished 处理响应
    connect(&m_netManager, &QNetworkAccessManager::finished,
            this, &ApiService::onReplyFinished);
}

// 发起注册请求
// 构建注册 API 的 HTTP POST 请求，发送用户名、邮箱和密码给服务器
// 请求成功后，服务器会返回包含 token 和用户信息的 JSON 响应
void ApiService::registerUser(const QString &username, const QString &email, const QString &password)
{
    // 构建注册 API 的完整 URL
    QUrl url(baseUrl() + "/api/auth/register");
    // 创建网络请求对象
    QNetworkRequest request(url);
    // 设置请求头，指定发送的数据格式为 JSON
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建要发送的 JSON 数据体
    QJsonObject json;
    json["username"] = username; // 添加用户名字段
    json["email"] = email;       // 添加邮箱字段
    json["password"] = password; // 添加密码字段

    // 将 JSON 对象转换为 JSON 文档，再转换为字节数组
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(); // 这个 data 就是即将发送的请求体

    qDebug() << "Sending POST request to" << url.toString() << "with data:" << data; // 调试
    // 使用网络管理器发起 POST 请求，将 data 作为请求体发送
    m_netManager.post(request, data);
}

// 发起登录请求
// 构建登录 API 的 HTTP POST 请求，发送用户名和密码给服务器
// 请求成功后，服务器会返回包含 token 和用户信息的 JSON 响应
void ApiService::login(const QString &username, const QString &password)
{

    QUrl url(baseUrl() + "/api/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建要发送的 JSON 数据体
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    // 将 JSON 对象转换为 JSON 文档，再转换为字节数组
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(); // 这个 data 就是即将发送的请求体

    qDebug() << "Sending POST request to" << url.toString() << "with data:" << data; // 调试
    m_netManager.post(request, data);
}

//搜索用户
void ApiService::searchUsers(const QString &query)
{
    if (query.trimmed().length() < 2) {
        emit searchUsersFailed("Query too short.");
        return;
    }

    QUrl url(baseUrl() + "/api/friends/search");
    QUrlQuery queryItems;
    queryItems.addQueryItem("q", query);
    url.setQuery(queryItems);

    QNetworkRequest request(url);
    QNetworkReply* reply = m_netManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                emit searchUsersSuccess(doc.array());
            } else {
                emit searchUsersFailed("Invalid response format.");
            }
        } else {
            emit searchUsersFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// 新增：发送好友请求
void ApiService::sendFriendRequest(int friendId) {
    QUrl url(baseUrl() + "/api/friends/request");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["friendId"] = friendId;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_netManager.post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit sendFriendRequestSuccess(doc.object());
            } else {
                emit sendFriendRequestFailed("Invalid response format.");
            }
        } else {
            emit sendFriendRequestFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}
// 新增：获取好友列表
void ApiService::getFriends() {
    QUrl url(baseUrl() + "/api/friends");
    QNetworkRequest request(url);
    SessionManager& session = SessionManager::instance();
    request.setRawHeader("Authorization", ("Bearer " + session.token()).toUtf8());

    QNetworkReply* reply = m_netManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                emit getFriendsSuccess(doc.array());
            } else {
                emit getFriendsFailed("Invalid response format.");
            }
        } else {
            emit getFriendsFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// 新增：获取好友请求
void ApiService::getFriendRequests() {
    QUrl url(baseUrl() + "/api/friends/requests");
    QNetworkRequest request(url);
    SessionManager& session = SessionManager::instance();
    request.setRawHeader("Authorization", ("Bearer " + session.token()).toUtf8());

    QNetworkReply* reply = m_netManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                emit getFriendRequestsSuccess(doc.array());
            } else {
                emit getFriendRequestsFailed("Invalid response format.");
            }
        } else {
            emit getFriendRequestsFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// 新增：接受好友请求
void ApiService::acceptFriendRequest(int requesterId) {
    QUrl url(baseUrl() + "/api/friends/accept");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    SessionManager& session = SessionManager::instance();
    request.setRawHeader("Authorization", ("Bearer " + session.token()).toUtf8());

    QJsonObject json;
    json["requesterId"] = requesterId;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_netManager.post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit acceptFriendRequestSuccess(doc.object());
            } else {
                emit acceptFriendRequestFailed("Invalid response format.");
            }
        } else {
            emit acceptFriendRequestFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// 新增：拒绝好友请求
void ApiService::rejectFriendRequest(int requesterId) {
    QUrl url(baseUrl() + "/api/friends/reject");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    SessionManager& session = SessionManager::instance();
    request.setRawHeader("Authorization", ("Bearer " + session.token()).toUtf8());

    QJsonObject json;
    json["requesterId"] = requesterId;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_netManager.post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit rejectFriendRequestSuccess();
        } else {
            emit rejectFriendRequestFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// 获取当前用户的详细信息
// 在请求头中添加认证 token，向服务器请求当前登录用户的信息
void ApiService::fetchUserInfo()
{
    // 从 SessionManager 获取当前用户的认证 token
    QString token = SessionManager::instance().token();
    // 如果没有 token（即未登录），则不发送请求
    if (token.isEmpty()) return;

    // 构建获取用户信息的 API URL
    QUrl url(baseUrl() + "/api/users/me");
    // 创建网络请求对象
    QNetworkRequest request(url);
    // 设置请求头中的 Authorization 字段，使用 Bearer Token 认证方式
    // 注意：这里使用了 toUtf8()，因为 setRawHeader 接受的是 QByteArray
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    // 使用网络管理器发起 GET 请求来获取用户信息
    m_netManager.get(request);
}

// 获取当前用户的好友列表
// 在请求头中添加认证 token，向服务器请求当前登录用户的好友列表
void ApiService::fetchFriends()
{
    // 从 SessionManager 获取当前用户的认证 token
    QString token = SessionManager::instance().token();
    // 如果没有 token（即未登录），则不发送请求
    if (token.isEmpty()) return;

    // 构建获取好友列表的 API URL
    QUrl url(baseUrl() + "/api/friends");
    // 创建网络请求对象
    QNetworkRequest request(url);
    // 设置请求头中的 Authorization 字段，使用 Bearer Token 认证方式
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    // 使用网络管理器发起 GET 请求来获取好友列表
    m_netManager.get(request);
}

// 处理网络请求完成的回调函数（槽函数）
// 此函数在每次网络请求完成时被调用（无论是成功还是失败）
void ApiService::onReplyFinished(QNetworkReply *reply)
{
    // 检查请求过程中是否有网络错误
    if (reply->error() != QNetworkReply::NoError) {
        // 如果有错误，记录警告信息
        qWarning() << "Network error:" << reply->errorString();
        // 发出登录失败信号，传递错误信息
        emit loginFailed(reply->errorString());
        // 释放 reply 对象占用的内存（Qt 中推荐的做法）
        reply->deleteLater();
        return; // 结束函数执行
    }

    // 读取服务器返回的原始数据
    QByteArray data = reply->readAll();
    // 尝试将返回的数据解析为 JSON 文档
    QJsonDocument doc = QJsonDocument::fromJson(data);
    // 检查解析结果是否有效（必须是一个 JSON 对象）
    if (!doc.isObject()) {
        // 如果不是有效的 JSON 对象，发出登录失败信号
        emit loginFailed("Invalid response format");
        // 释放 reply 对象
        reply->deleteLater();
        return; // 结束函数执行
    }

    // 将 JSON 文档转换为 JSON 对象，以便访问其键值对
    QJsonObject json = doc.object();

    // 获取原始请求的 URL，用于判断是哪个 API 调用的响应
    QUrl requestUrl = reply->request().url();
    // 获取请求路径部分
    QString path = requestUrl.path();

    // 根据请求路径判断是哪种类型的响应，并进行相应的处理
    if (path == "/api/auth/login") {
        // 处理登录 API 的响应
        if (json.contains("token")) {
            // 如果响应包含 token 字段，表示登录成功
            QString token = json["token"].toString(); // 提取 token
            // 提取用户信息对象
            QJsonObject user = json["user"].toObject();
            int userId = user["id"].toInt();        // 提取用户 ID
            QString username = user["username"].toString(); // 提取用户名

            // 使用 SessionManager 更新和保存会话信息（token 和用户信息）
            SessionManager::instance().setToken(token);
            SessionManager::instance().setUserInfo(userId, username);
            SessionManager::instance().save(); // 持久化保存

            // 发出登录成功的信号，传递用户对象和 token
            emit loginSuccess(user, token);
        } else {
            // 如果响应不包含 token，说明登录失败
            // 尝试提取服务器返回的错误消息，如果不存在则使用默认消息
            emit loginFailed(json["message"].toString("Login failed"));
        }
    }
    else if (path == "/api/auth/register") {
        // 处理注册 API 的响应
        if (json.contains("token")) {
            // 注册成功，响应中包含 token 和 user
            QString token = json["token"].toString(); // 提取 token
            QJsonObject user = json["user"].toObject(); // 提取用户信息对象
            // 注意：user 对象中通常包含 id, username, email 等字段
            // 可以根据需要提取特定字段
            // int userId = user["id"].toInt(); // 如果需要用户 ID
            // QString username = user["username"].toString(); // 如果需要用户名

            // --- 可选：保存会话信息 ---
            // 如果注册后立即需要登录（比如前端自动登录），可以保存 token 和用户信息
            // SessionManager::instance().setToken(token);
            // SessionManager::instance().setUserInfo(userId, username); // 需要从 user 中提取
            // SessionManager::instance().save();

            // 发出注册成功的信号
            emit registerSuccess(user, token); // 传递 user 和 token
        } else {
            // 注册失败，尝试提取服务器返回的错误消息
            QString errorMessage = json["message"].toString("Registration failed"); // 提供默认消息
            emit registerFailed(errorMessage);
        }
    }
    else if (path == "/api/users/me") {
        // 处理获取用户信息 API 的响应
        // 直接发出用户信息获取成功的信号，将完整的 JSON 对象传递出去
        emit userInfoFetched(json);
    } else if (path == "/api/friends") {
        // 处理获取好友列表 API 的响应
        // 服务器返回的是 {"data": [...]}
        if (json.contains("data") && json["data"].isArray()) {
            QJsonArray friends = json["data"].toArray();
            emit friendsFetched(friends);
        } else {
            qWarning() << "Unexpected friends response structure";

        }
    }

    // 无论处理成功与否，都释放 reply 对象占用的内存
    reply->deleteLater();
}
