#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "user_manage.hpp"
#include "adminwindow.h"
#include "studentwindow.h"
#include "teacherwindow.h"
#include "registerdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class MatchingManager;

class LoginWindow : public QMainWindow {
    Q_OBJECT

public:
    LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();

signals:
    void loggedOut();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void handleLogout();
    void reconnectRegisterButton();

private:
    Ui::LoginWindow* ui;
    std::unique_ptr<UserManage> userManage;
    std::unique_ptr<MatchingManager> matchingManager;
    AdminWindow* adminWindow;
};

#endif // LOGINWINDOW_H