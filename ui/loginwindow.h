#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "user_manage.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void on_registerButton_clicked();
    void on_loginButton_clicked();

private:
    Ui::LoginWindow *ui;
    std::unique_ptr<UserManage> userManage;
};
#endif // LOGINWINDOW_H