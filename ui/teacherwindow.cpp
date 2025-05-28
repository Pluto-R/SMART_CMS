#include "teacherwindow.h"
#include "ui_teacherwindow.h"
#include "scoreanalysisdialog.h"
#include "loginwindow.h"

TeacherWindow::TeacherWindow(const QString &username, std::unique_ptr<UserManage> userManage, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TeacherWindow)
    , username(username)
    , userManage(std::move(userManage))
{
    ui->setupUi(this);
    setWindowTitle("教师界面 - " + username);
    
    scoreAnalysisDialog = new ScoreAnalysisDialog(userManage.get(), username, this);
}

TeacherWindow::~TeacherWindow()
{
    delete ui;
    delete scoreAnalysisDialog;
}

void TeacherWindow::on_analyzeScoresButton_clicked()
{
    scoreAnalysisDialog->show();
}

void TeacherWindow::on_logoutButton_clicked()
{
    auto *loginWindow = new LoginWindow();
    loginWindow->show();
    this->close();
}