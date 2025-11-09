#include "ui/chatmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatMainWindow w;
    w.show();
    return a.exec();
}
