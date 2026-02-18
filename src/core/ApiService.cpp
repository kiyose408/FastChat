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
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>


ApiService::ApiService(QObject *parent)
    : QObject(parent) // 调用父类 QObject 的构造函数
{
    // 移除全局的 finished 信号连接，让每个请求自己处理自己的响应
    // 这样可以避免响应被处理两次的问题
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
    QNetworkReply* reply = m_netManager.post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject json = doc.object();
                if (json.contains("token")) {
                    QString token = json["token"].toString();
                    QJsonObject user = json["user"].toObject();
                    emit registerSuccess(user, token);
                } else {
                    emit registerFailed(json["message"].toString("Registration failed"));
                }
            } else {
                emit registerFailed("Invalid response format");
            }
        } else {
            emit registerFailed(reply->errorString());
        }
        reply->deleteLater();
    });
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
    QNetworkReply* reply = m_netManager.post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject json = doc.object();
                if (json.contains("token")) {
                    QString token = json["token"].toString();
                    QJsonObject user = json["user"].toObject();
                    int userId = user["id"].toInt();
                    QString username = user["username"].toString();

                    SessionManager::instance().setToken(token);
                    SessionManager::instance().setUserInfo(userId, username);
                    SessionManager::instance().save();

                    emit loginSuccess(user, token);
                } else {
                    emit loginFailed(json["message"].toString("Login failed"));
                }
            } else {
                emit loginFailed("Invalid response format");
            }
        } else {
            emit loginFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

//搜索用户
void ApiService::searchUsers(const QString &query)
{
    if (query.trimmed().length() < 2) {
        emit searchUsersFailed("Query too short.");
        return;
    }

    // 从 SessionManager 获取当前用户的认证 token
    QString token = SessionManager::instance().token();
    // 如果没有 token（即未登录），则不发送请求
    if (token.isEmpty()) {
        emit searchUsersFailed("Not authenticated.");
        return;
    }

    QUrl url(baseUrl() + "/api/friends/search");
    QUrlQuery queryItems;
    queryItems.addQueryItem("q", query);
    url.setQuery(queryItems);

    QNetworkRequest request(url);
    // 设置请求头中的 Authorization 字段，使用 Bearer Token 认证方式
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    QNetworkReply* reply = m_netManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Received search response data:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                qDebug() << "Response is array, length:" << doc.array().size();
                emit searchUsersSuccess(doc.array());
            } else {
                qDebug() << "Response is not array, isObject:" << doc.isObject() << "isNull:" << doc.isNull();
                emit searchUsersFailed("Invalid response format.");
            }
        } else {
            emit searchUsersFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// 新增：发送好友请求
void ApiService::sendFriendRequest(int friendId, const QString& note) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit sendFriendRequestFailed("Not authenticated.");
        return;
    }

    QUrl url(baseUrl() + "/api/friends/request");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject json;
    json["friendId"] = friendId;
    json["note"] = note;
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
            qDebug() << "Get friends response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("friends") && obj["friends"].isArray()) {
                    emit getFriendsSuccess(obj["friends"].toArray());
                } else {
                    emit getFriendsFailed("Invalid response format: missing friends array.");
                }
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

void ApiService::deleteFriend(int friendId) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit deleteFriendFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/friends/" + QString::number(friendId));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QNetworkReply* reply = m_netManager.deleteResource(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit deleteFriendSuccess();
        } else {
            emit deleteFriendFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::updateFriendNote(int friendId, const QString& note) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit updateFriendNoteFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/friends/" + QString::number(friendId) + "/note");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QJsonObject json;
    json["note"] = note;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    QNetworkReply* reply = m_netManager.put(request, data);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit updateFriendNoteSuccess(doc.object());
            } else {
                emit updateFriendNoteFailed("Invalid response format.");
            }
        } else {
            emit updateFriendNoteFailed(reply->errorString());
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
    QNetworkReply* reply = m_netManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit userInfoFetched(doc.object());
            } else {
                qWarning() << "Invalid user info response format";
            }
        } else {
            qWarning() << "Network error fetching user info:" << reply->errorString();
        }
        reply->deleteLater();
    });
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
    QNetworkReply* reply = m_netManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject() && doc.object().contains("data")) {
                QJsonArray friends = doc.object()["data"].toArray();
                emit friendsFetched(friends);
            } else {
                qWarning() << "Unexpected friends response structure";
            }
        } else {
            qWarning() << "Network error fetching friends:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void ApiService::sendMessage(int recipientId, const QString& content) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit sendMessageFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/messages/send");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QJsonObject json;
    json["recipientId"] = recipientId;
    json["content"] = content;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    QNetworkReply* reply = m_netManager.post(request, data);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Send message response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit sendMessageSuccess(doc.object());
            } else {
                emit sendMessageFailed("Invalid response format.");
            }
        } else {
            emit sendMessageFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::getConversation(int recipientId) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit getConversationFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/messages/conversation/" + QString::number(recipientId));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QNetworkReply* reply = m_netManager.get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Get conversation response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                emit getConversationSuccess(doc.array());
            } else {
                emit getConversationFailed("Invalid response format.");
            }
        } else {
            emit getConversationFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::uploadImage(const QString& filePath) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit uploadImageFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/upload/image");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/jpeg");
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\"; filename=\"image.jpg\""));
    
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadImageFailed("Cannot open file.");
        delete multiPart;
        delete file;
        return;
    }
    
    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(imagePart);
    
    QNetworkReply* reply = m_netManager.post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Upload image response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit uploadImageSuccess(doc.object());
            } else {
                emit uploadImageFailed("Invalid response format.");
            }
        } else {
            emit uploadImageFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::markMessagesAsRead(int senderId) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit markMessagesAsReadFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/messages/mark-read");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QJsonObject json;
    json["senderId"] = senderId;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    QNetworkReply* reply = m_netManager.post(request, data);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, senderId]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit markMessagesAsReadSuccess(senderId);
        } else {
            emit markMessagesAsReadFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::uploadFile(const QString& filePath) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit uploadFileFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/upload/file");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadFileFailed("Cannot open file.");
        delete multiPart;
        delete file;
        return;
    }
    
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);
    
    QNetworkReply* reply = m_netManager.post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Upload file response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit uploadFileSuccess(doc.object());
            } else {
                emit uploadFileFailed("Invalid response format.");
            }
        } else {
            emit uploadFileFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::uploadAvatar(const QString& filePath) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit uploadAvatarFailed("Not authenticated.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/upload/avatar");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    QString ext = fileInfo.suffix().toLower();
    
    QString mimeType = "image/jpeg";
    if (ext == "png") mimeType = "image/png";
    else if (ext == "gif") mimeType = "image/gif";
    else if (ext == "webp") mimeType = "image/webp";
    
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"avatar\"; filename=\"%1\"").arg(fileName)));
    
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadAvatarFailed("Cannot open file.");
        delete multiPart;
        delete file;
        return;
    }
    
    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(imagePart);
    
    QNetworkReply* reply = m_netManager.post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Upload avatar response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                emit uploadAvatarSuccess(doc.object());
            } else {
                emit uploadAvatarFailed("Invalid response format.");
            }
        } else {
            emit uploadAvatarFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}
