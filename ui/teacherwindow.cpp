#include "teacherwindow.h"
#include "ui_teacherwindow.h"
#include "scoreanalysisdialog.h"
#include "loginwindow.h"
#include <QMessageBox>

TeacherWindow::TeacherWindow(const QString &username, std::unique_ptr<UserManage> userManage, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TeacherWindow)
    , username(username)
    , userManage(std::move(userManage))
    , scoreAnalysisDialog(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("教师界面 - " + username);
    
    // 延迟创建对话框，直到需要时
}

TeacherWindow::~TeacherWindow()
{
    delete ui;
    delete scoreAnalysisDialog;
}

void TeacherWindow::on_analyzeScoresButton_clicked()
{
    if (!scoreAnalysisDialog) {
        scoreAnalysisDialog = new ScoreAnalysisDialog(userManage.get(), username, this);
    }
    scoreAnalysisDialog->show();
}

void TeacherWindow::on_logoutButton_clicked()
{
    auto *loginWindow = new LoginWindow();
    loginWindow->show();
    this->close();
}