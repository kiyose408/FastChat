#ifndef CHATMAINWINDOW_H
#define CHATMAINWINDOW_H

#include <QMainWindow>
class FriendModel;
QT_BEGIN_NAMESPACE
namespace Ui {
class ChatMainWindow;
}
QT_END_NAMESPACE

class ChatMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ChatMainWindow(QWidget *parent = nullptr);
    ~ChatMainWindow();
private slots:
    void onFriendClicked(const QModelIndex &index);
private:
    Ui::ChatMainWindow *ui;
    FriendModel *m_friendModel;
};
#endif // CHATMAINWINDOW_H
