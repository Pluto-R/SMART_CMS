#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "studentwindow.h"
#include "teacherwindow.h"
#include <QMessageBox>
#include <QDebug>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::LoginWindow),
      userManage(std::make_unique<UserManage>(
          "/home/jqz/SMART_CMS/users.txt",
          "/home/jqz/SMART_CMS/teacher_info.txt"))
{
    ui->setupUi(this);
    ui->userTypeComboBox->addItem("学生", "0");
    ui->userTypeComboBox->addItem("老师", "1");
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_loginButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString userType = ui->userTypeComboBox->currentData().toString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示信息", "用户名和密码不能为空");
        return;
    }

    bool success = userManage->Login(userType.toStdString(),
                                     username.toStdString(),
                                     password.toStdString());

    if (success) {
        qDebug() << "Success to login!";
        if (userType == "0") {
            auto *studentWindow = new StudentWindow(username, userManage.get(), nullptr); // 确保父对象为 nullptr
            studentWindow->show();
            this->close();
        } else {
            auto *teacherWindow = new TeacherWindow(username, userManage.get(), nullptr); // 确保父对象为 nullptr
            teacherWindow->show();
            this->close();
        }
    } else {
        QMessageBox::warning(this, "提示信息", "用户名或密码错误");
    }
}

void LoginWindow::on_registerButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString userType = ui->userTypeComboBox->currentData().toString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "用户名和密码不能为空");
        return;
    }

    bool success = userManage->Registered(userType.toStdString(),
                                       username.toStdString(),
                                       password.toStdString());

    if (success) {
        QMessageBox::information(this, "注册成功", "用户注册成功，请登录");
    } else {
        QMessageBox::warning(this, "注册失败", "用户名已存在");
    }
}