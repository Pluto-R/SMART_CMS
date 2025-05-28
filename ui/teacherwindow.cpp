#include "teacherwindow.h"
#include "ui_teacherwindow.h"
#include "scoreanalysisdialog.h"
#include "loginwindow.h"

TeacherWindow::TeacherWindow(const QString &username, UserManage *userManage, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::TeacherWindow), username(username), userManage(userManage)
{
    ui->setupUi(this);
    setWindowTitle("教师界面 - " + username);
    scoreAnalysisDialog = nullptr; // 延迟初始化
}

TeacherWindow::~TeacherWindow()
{
    delete ui;
    delete scoreAnalysisDialog;
}

void TeacherWindow::on_analyzeScoresButton_clicked()
{
    if (!scoreAnalysisDialog) {
        scoreAnalysisDialog = new ScoreAnalysisDialog(userManage, username, this);
    }
    scoreAnalysisDialog->show();
}

void TeacherWindow::on_logoutButton_clicked()
{
    auto *loginWindow = new LoginWindow(nullptr); // 确保无父对象
    loginWindow->show();
    this->close();
}