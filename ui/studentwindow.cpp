#include "studentwindow.h"
#include "ui_studentwindow.h"
#include "matchingdialog.h"
#include "scoreanalysisdialog.h"
#include "scoreinputdialog.h"
#include "loginwindow.h"

StudentWindow::StudentWindow(const QString &username, std::unique_ptr<UserManage> userManage, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StudentWindow)
    , username(username)
    , userManage(std::move(userManage))
{
    ui->setupUi(this);
    setWindowTitle("学生界面 - " + username);
    
    // 初始化对话框
    matchingDialog = new MatchingDialog(userManage.get(), username, this);
    scoreAnalysisDialog = new ScoreAnalysisDialog(userManage.get(), username, this);
    scoreInputDialog = new ScoreInputDialog(userManage.get(), username, this);
}

StudentWindow::~StudentWindow()
{
    delete ui;
    delete matchingDialog;
    delete scoreAnalysisDialog;
    delete scoreInputDialog;
}

void StudentWindow::on_matchTeacherButton_clicked()
{
    matchingDialog->show();
}

void StudentWindow::on_analyzeScoresButton_clicked()
{
    scoreAnalysisDialog->show();
}

void StudentWindow::on_inputScoreButton_clicked()
{
    scoreInputDialog->show();
}

void StudentWindow::on_logoutButton_clicked()
{
    auto *loginWindow = new LoginWindow();
    loginWindow->show();
    this->close();
}