#include "loginwindow.h"
#include <QApplication>
#include <QFile>        // 添加这个头文件
#include <QIODevice>    // 添加这个头文件

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 加载样式表
    QFile styleFile(":/styles/style.css");
    if(styleFile.open(QIODevice::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    }
    
    LoginWindow w;
    w.show();
    return a.exec();
}