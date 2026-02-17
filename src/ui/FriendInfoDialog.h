#ifndef FRIENDINFODIALOG_H
#define FRIENDINFODIALOG_H

#include <QDialog>
#include "core/ApiService.h"

namespace Ui {
class FriendInfoDialog;
}

class FriendInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FriendInfoDialog(ApiService* apiService, int friendId, const QString& friendName, QWidget *parent = nullptr);
    ~FriendInfoDialog();

private slots:
    void onSaveNoteBtnClicked();
    void onDeleteFriendBtnClicked();
    void onUpdateNoteSuccess(const QJsonObject& result);
    void onUpdateNoteFailed(const QString& error);
    void onDeleteFriendSuccess();
    void onDeleteFriendFailed(const QString& error);

private:
    Ui::FriendInfoDialog *ui;
    ApiService* m_apiService;
    int m_friendId;
    QString m_friendName;
};

#endif // FRIENDINFODIALOG_H
