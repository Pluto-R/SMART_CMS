#include <QApplication>
#include "loginwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}