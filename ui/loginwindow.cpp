#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "studentwindow.h"
#include "teacherwindow.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    setWindowTitle("智能家教匹配系统 - 登录");
    
    userManage = std::make_unique<UserManage>(
        "/home/jqz/Desktop/Smart_CMS/users.txt",
        "/home/jqz/Desktop/Smart_CMS/teacher_info.txt"
    );
    
    // 设置用户类型下拉框
    ui->userTypeComboBox->addItem("学生", "0");
    ui->userTypeComboBox->addItem("老师", "1");
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_registerButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString userType = ui->userTypeComboBox->currentData().toString();
    
    if(username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "用户名和密码不能为空");
        return;
    }
    
    userManage->Registered(userType.toStdString(), 
                         username.toStdString(), 
                         password.toStdString());
    
    QMessageBox::information(this, "注册成功", "用户注册成功！");
}

void LoginWindow::on_loginButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString userType = ui->userTypeComboBox->currentData().toString();
    
    if(username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "用户名和密码不能为空");
        return;
    }
    
    bool success = userManage->Login(userType.toStdString(),
                                   username.toStdString(),
                                   password.toStdString());
    
    if(success) {
        if(userType == "0") {
            // 学生登录
            auto *studentWindow = new StudentWindow(username, std::move(userManage));
            studentWindow->show();
            this->close();
        } else {
            // 老师登录
            auto *teacherWindow = new TeacherWindow(username, std::move(userManage));
            teacherWindow->show();
            this->close();
        }
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}