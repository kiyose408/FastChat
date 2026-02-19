#include "ApiService.h"
#include "SessionManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>

ApiService::ApiService(QObject *parent)
    : QObject(parent)
    , m_retryPolicy(new RetryPolicy(RetryConfig(), this))
{
}

QString ApiService::generateRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ApiService::setRetryConfig(const RetryConfig& config)
{
    m_retryPolicy->setConfig(config);
}

void ApiService::setRequestTimeout(int ms)
{
    m_timeoutMs = ms;
}

bool ApiService::shouldRetryRequest(QNetworkReply* reply, int retryCount)
{
    if (!m_retryPolicy->canRetry(retryCount)) {
        return false;
    }
    
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError error = reply->error();
    
    if (statusCode == 401) {
        return false;
    }
    
    return m_retryPolicy->shouldRetry(error, statusCode);
}

void ApiService::retryRequest(const QString& requestId)
{
    if (!m_pendingRequests.contains(requestId)) {
        return;
    }
    
    PendingRequest& req = m_pendingRequests[requestId];
    int delay = m_retryPolicy->calculateDelay(req.retryCount);
    req.retryCount++;
    
    qDebug() << "Retrying request" << req.apiName 
             << "attempt" << req.retryCount 
             << "after" << delay << "ms";
    
    QTimer::singleShot(delay, this, [this, requestId]() {
        executeRequest(requestId);
    });
}

void ApiService::executeRequest(const QString& requestId)
{
    if (!m_pendingRequests.contains(requestId)) {
        return;
    }
    
    PendingRequest& req = m_pendingRequests[requestId];
    
    QNetworkReply* reply = nullptr;
    
    if (req.method == "GET") {
        reply = m_netManager.get(req.request);
    } else if (req.method == "POST") {
        reply = m_netManager.post(req.request, req.data);
    } else if (req.method == "PUT") {
        reply = m_netManager.put(req.request, req.data);
    } else if (req.method == "DELETE") {
        reply = m_netManager.deleteResource(req.request);
    }
    
    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [this, requestId, reply]() {
            handleReplyFinished(requestId, reply);
        });
    }
}

void ApiService::handleReplyFinished(const QString& requestId, QNetworkReply* reply)
{
    if (!m_pendingRequests.contains(requestId)) {
        reply->deleteLater();
        return;
    }
    
    PendingRequest& req = m_pendingRequests[requestId];
    
    if (reply->error() != QNetworkReply::NoError && shouldRetryRequest(reply, req.retryCount)) {
        retryRequest(requestId);
        reply->deleteLater();
        return;
    }
    
    m_pendingRequests.remove(requestId);
}

void ApiService::registerUser(const QString &username, const QString &email, const QString &password)
{
    QUrl url(baseUrl() + "/api/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["email"] = email;
    json["password"] = password;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    qDebug() << "Sending POST request to" << url.toString() << "with data:" << data;
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

void ApiService::login(const QString &username, const QString &password)
{
    QUrl url(baseUrl() + "/api/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    qDebug() << "Sending POST request to" << url.toString() << "with data:" << data;
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

void ApiService::searchUsers(const QString &query)
{
    if (query.trimmed().length() < 2) {
        emit searchUsersFailed("Query too short.");
        return;
    }

    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit searchUsersFailed("Not authenticated.");
        return;
    }

    QUrl url(baseUrl() + "/api/friends/search");
    QUrlQuery queryItems;
    queryItems.addQueryItem("q", query);
    url.setQuery(queryItems);

    QNetworkRequest request(url);
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

void ApiService::fetchUserInfo()
{
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) return;

    QUrl url(baseUrl() + "/api/users/me");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

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

void ApiService::fetchFriends()
{
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) return;

    QUrl url(baseUrl() + "/api/friends");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

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

void ApiService::searchMessages(const QString& query) {
    QString token = SessionManager::instance().token();
    if (token.isEmpty()) {
        emit searchMessagesFailed("Not authenticated.");
        return;
    }
    
    if (query.trimmed().isEmpty()) {
        emit searchMessagesFailed("Search query is empty.");
        return;
    }
    
    QUrl url(baseUrl() + "/api/messages/search");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query.trimmed());
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    
    QNetworkReply* reply = m_netManager.get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, query]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Search messages response:" << data;
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("results") && obj["results"].isArray()) {
                    emit searchMessagesSuccess(obj["results"].toArray(), query);
                } else {
                    emit searchMessagesFailed("Invalid response format.");
                }
            } else {
                emit searchMessagesFailed("Invalid response format.");
            }
        } else {
            emit searchMessagesFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}
