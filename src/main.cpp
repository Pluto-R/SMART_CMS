#include <QApplication>
#include "loginwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false); // 防止最后一个窗口关闭时退出
    LoginWindow w;
    w.show();
    return app.exec();
}