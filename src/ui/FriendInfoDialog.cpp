#include "FriendInfoDialog.h"
#include "ui_FriendInfoDialog.h"
#include <QMessageBox>

FriendInfoDialog::FriendInfoDialog(ApiService* apiService, int friendId, const QString& friendName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FriendInfoDialog)
    , m_apiService(apiService)
    , m_friendId(friendId)
    , m_friendName(friendName)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    
    ui->usernameLabel->setText(friendName);
    ui->avatarLabel->setText(friendName.isEmpty() ? "?" : friendName.left(1).toUpper());
    
    connect(ui->saveNoteBtn, &QPushButton::clicked, this, &FriendInfoDialog::onSaveNoteBtnClicked);
    connect(ui->deleteFriendBtn, &QPushButton::clicked, this, &FriendInfoDialog::onDeleteFriendBtnClicked);
    
    connect(m_apiService, &ApiService::updateFriendNoteSuccess, this, &FriendInfoDialog::onUpdateNoteSuccess);
    connect(m_apiService, &ApiService::updateFriendNoteFailed, this, &FriendInfoDialog::onUpdateNoteFailed);
    connect(m_apiService, &ApiService::deleteFriendSuccess, this, &FriendInfoDialog::onDeleteFriendSuccess);
    connect(m_apiService, &ApiService::deleteFriendFailed, this, &FriendInfoDialog::onDeleteFriendFailed);
}

FriendInfoDialog::~FriendInfoDialog()
{
    delete ui;
}

void FriendInfoDialog::onSaveNoteBtnClicked()
{
    QString note = ui->noteLineEdit->text().trimmed();
    m_apiService->updateFriendNote(m_friendId, note);
}

void FriendInfoDialog::onDeleteFriendBtnClicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除", 
                                   QString("确定要删除好友 \"%1\" 吗？\n删除后将无法恢复。").arg(m_friendName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_apiService->deleteFriend(m_friendId);
    }
}

void FriendInfoDialog::onUpdateNoteSuccess(const QJsonObject& result)
{
    qDebug() << "备注更新成功:" << result["message"].toString();
    QMessageBox::information(this, "成功", "备注已更新");
}

void FriendInfoDialog::onUpdateNoteFailed(const QString& error)
{
    qDebug() << "备注更新失败:" << error;
    QMessageBox::warning(this, "失败", "备注更新失败: " + error);
}

void FriendInfoDialog::onDeleteFriendSuccess()
{
    qDebug() << "好友删除成功";
    QMessageBox::information(this, "成功", "好友已删除");
    accept();
}

void FriendInfoDialog::onDeleteFriendFailed(const QString& error)
{
    qDebug() << "好友删除失败:" << error;
    QMessageBox::warning(this, "失败", "删除好友失败: " + error);
}
