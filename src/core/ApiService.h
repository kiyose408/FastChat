#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QHttpMultiPart>

class ApiService : public QObject
{
    Q_OBJECT

public:

    explicit ApiService(QObject *parent = nullptr);

    void registerUser(const QString &username, const QString &email, const QString &password);
    void login(const QString& username, const QString& password);

    void searchUsers(const QString& query);
    void sendFriendRequest(int friendId, const QString& note = "");
    void getFriends();
    void getFriendRequests();
    void acceptFriendRequest(int requesterId);
    void rejectFriendRequest(int requesterId);
    void deleteFriend(int friendId);
    void updateFriendNote(int friendId, const QString& note);

    void fetchUserInfo();
    void fetchFriends();
    
    void sendMessage(int recipientId, const QString& content);
    void getConversation(int recipientId);
    void markMessagesAsRead(int senderId);
    
    void uploadImage(const QString& filePath);
    void uploadFile(const QString& filePath);

signals:

    void registerSuccess(const QJsonObject &user, const QString &token);
    void registerFailed(const QString &error);

    void loginSuccess(const QJsonObject& user, const QString& token);
    void loginFailed(const QString& error);

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
    void deleteFriendSuccess();
    void deleteFriendFailed(const QString& error);
    void updateFriendNoteSuccess(const QJsonObject& result);
    void updateFriendNoteFailed(const QString& error);

    void userInfoFetched(const QJsonObject& user);
    void friendsFetched(const QJsonArray& friends);
    
    void sendMessageSuccess(const QJsonObject& message);
    void sendMessageFailed(const QString& error);
    void getConversationSuccess(const QJsonArray& messages);
    void getConversationFailed(const QString& error);
    void markMessagesAsReadSuccess(int senderId);
    void markMessagesAsReadFailed(const QString& error);
    
    void uploadImageSuccess(const QJsonObject& result);
    void uploadImageFailed(const QString& error);
    void uploadFileSuccess(const QJsonObject& result);
    void uploadFileFailed(const QString& error);



private:
    QNetworkAccessManager m_netManager;

    QString baseUrl() const { return "http://localhost:3000"; }
};

#endif // APISERVICE_H
