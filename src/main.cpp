#include "ui/chatmainwindow.h"
#include "utils/logger.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("KIYOSE");
    QApplication::setApplicationName("FastChat");
    QApplication::setApplicationVersion("0.1.0-test");
    QApplication app(argc, argv);
    Logger::init(); //初始化日志
    Logger::info("Application started");
    ChatMainWindow w;
    w.show();
    int result = app.exec();
    Logger::info("Application exited");
    return result;
}
