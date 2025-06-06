#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "studentwindow.h"
#include "teacherwindow.h"
#include "adminwindow.h"
#include "registerdialog.h"
#include "matching.hpp"
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QTimer>

LoginWindow::LoginWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::LoginWindow), adminWindow(nullptr) {
    qDebug() << "开始初始化 LoginWindow";
    ui->setupUi(this);
    qDebug() << "UI 设置完成";
    
    qDebug() << "初始化 UserManage";
    userManage = std::make_unique<UserManage>(
        "/home/jqz/SMART_CMS/users.txt",
        "/home/jqz/SMART_CMS/teacher_info.txt",
        "/home/jqz/SMART_CMS/relationships.txt",
        "/home/jqz/SMART_CMS/student_records.txt",
        "/home/jqz/SMART_CMS/pending_teachers.txt");
    qDebug() << "UserManage 初始化完成";

    qDebug() << "初始化 MatchingManager";
    matchingManager = std::make_unique<MatchingManager>(*userManage);
    matchingManager->rebuildIndexes();
    qDebug() << "MatchingManager 初始化并重建索引完成";

    qDebug() << "设置用户类型选择框";
    ui->userTypeComboBox->addItem("学生", "0");
    ui->userTypeComboBox->addItem("老师", "1");
    ui->userTypeComboBox->addItem("管理员", "2");
    
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    qDebug() << "连接信号和槽";
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::on_loginButton_clicked, Qt::UniqueConnection);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginWindow::on_registerButton_clicked, Qt::UniqueConnection);
    qDebug() << "LoginWindow 初始化完成";
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::on_loginButton_clicked() {
    ui->loginButton->setEnabled(false);
    QTimer::singleShot(1000, this, [=]() { ui->loginButton->setEnabled(true); });

    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();
    std::string userTypeData = ui->userTypeComboBox->currentData().toString().toStdString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "用户名和密码不能为空");
        return;
    }

    std::string actualUserType;
    bool success = userManage->Login(username.toStdString(), password.toStdString(), actualUserType);

    if (success) {
        qDebug() << "登录成功！用户名：" << username << " 类型：" << QString::fromStdString(actualUserType);
        matchingManager->rebuildIndexes();
        this->hide();

        static StudentWindow* studentWindow = nullptr;
        static TeacherWindow* teacherWindow = nullptr;
        static AdminWindow* adminWindow = nullptr;

        if (actualUserType == "0") {
            if (studentWindow) {
                qDebug() << "StudentWindow 已存在，显示现有窗口";
                studentWindow->show();
            } else {
                qDebug() << "创建 StudentWindow";
                studentWindow = new StudentWindow(username, userManage.get(), *matchingManager, nullptr);
                connect(studentWindow, &StudentWindow::loggedOut, this, [=]() {
                    qDebug() << "StudentWindow 注销，销毁窗口";
                    studentWindow->deleteLater();
                    studentWindow = nullptr;
                    handleLogout();
                }, Qt::UniqueConnection);
                studentWindow->show();
            }
            if (teacherWindow) {
                teacherWindow->deleteLater();
                teacherWindow = nullptr;
            }
            if (adminWindow) {
                adminWindow->deleteLater();
                adminWindow = nullptr;
            }
        } else if (actualUserType == "1") {
            if (teacherWindow) {
                qDebug() << "TeacherWindow 已存在，显示现有窗口";
                teacherWindow->show();
            } else {
                qDebug() << "创建 TeacherWindow";
                teacherWindow = new TeacherWindow(username, userManage.get(), nullptr);
                connect(teacherWindow, &TeacherWindow::loggedOut, this, [=]() {
                    qDebug() << "TeacherWindow 注销，销毁窗口";
                    teacherWindow->deleteLater();
                    teacherWindow = nullptr;
                    handleLogout();
                }, Qt::UniqueConnection);
                teacherWindow->show();
            }
            if (studentWindow) {
                studentWindow->deleteLater();
                studentWindow = nullptr;
            }
            if (adminWindow) {
                adminWindow->deleteLater();
                adminWindow = nullptr;
            }
        } else if (actualUserType == "2") {
            if (adminWindow) {
                qDebug() << "AdminWindow 已存在，显示现有窗口";
                adminWindow->show();
            } else {
                qDebug() << "创建 AdminWindow";
                adminWindow = new AdminWindow(username, userManage.get(), nullptr);
                connect(adminWindow, &AdminWindow::loggedOut, this, [=]() {
                    qDebug() << "AdminWindow 注销，销毁窗口";
                    adminWindow->deleteLater();
                    adminWindow = nullptr;
                    handleLogout();
                }, Qt::UniqueConnection);
                adminWindow->show();
            }
            if (studentWindow) {
                studentWindow->deleteLater();
                studentWindow = nullptr;
            }
            if (teacherWindow) {
                teacherWindow->deleteLater();
                teacherWindow = nullptr;
            }
        }
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}

void LoginWindow::on_registerButton_clicked() {
    RegisterDialog dialog(userManage.get(), this);
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "注册成功", "注册申请已提交，请等待管理员审批（老师）或直接登录（学生）。");
    }
}

void LoginWindow::reconnectRegisterButton() {
    QTimer::singleShot(1000, this, [=]() {
        ui->registerButton->setEnabled(true);
        connect(ui->registerButton, &QPushButton::clicked, this, &LoginWindow::on_registerButton_clicked, Qt::UniqueConnection);
        qDebug() << "注册按钮信号重新连接";
    });
}

void LoginWindow::handleLogout() {
    qDebug() << "处理注销信号，显示 LoginWindow";
    this->show();
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->userTypeComboBox->setCurrentIndex(0);
}