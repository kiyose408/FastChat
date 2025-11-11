#include "ui/chatmainwindow.h"
#include "utils/logger.h"
#include "ui/logindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("KIYOSE");
    QApplication::setApplicationName("FastChat");
    QApplication::setApplicationVersion("0.1.0-test");
    QApplication app(argc, argv);
    Logger::init(); //初始化日志
    Logger::info("Application started");
    //创建并且显示登录对话框
    LoginDialog loginDialog;
    int result = loginDialog.exec();//模态运行
    //检查登录结果
    if (result == QDialog::Accepted) {
        // 3. 登录成功：创建并显示主窗口
        ChatMainWindow mainWindow;
        mainWindow.show();

        // 4. 进入主事件循环
        return app.exec();
    } else {
        // 用户点击了“取消”或关闭窗口
        return 0; // 正常退出
    }
/*    ChatMainWindow w;
    w.show();
    int result = app.exec();
    Logger::info("Application exited");
    return result;
*/
}
