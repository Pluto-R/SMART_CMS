#include "studentwindow.h"
#include "ui_studentwindow.h"
#include "matchingdialog.h"
#include "scoreanalysisdialog.h"
#include "scoreinputdialog.h"
#include "loginwindow.h"
#include <QDebug>

StudentWindow::StudentWindow(const QString &username, UserManage *userManage, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::StudentWindow), username(username), userManage(userManage),
      matchingDialog(nullptr), scoreAnalysisDialog(nullptr), scoreInputDialog(nullptr)
{
    qDebug() << "Initializing StudentWindow for user:" << username;
    ui->setupUi(this);
    setWindowTitle("学生界面 - " + username);
    qDebug() << "StudentWindow UI setup completed";
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
    if (!matchingDialog) {
        matchingDialog = new MatchingDialog(userManage, username, this);
    }
    matchingDialog->show();
}

void StudentWindow::on_analyzeScoresButton_clicked()
{
    if (!scoreAnalysisDialog) {
        scoreAnalysisDialog = new ScoreAnalysisDialog(userManage, username, this);
    }
    scoreAnalysisDialog->show();
}

void StudentWindow::on_inputScoreButton_clicked()
{
    if (!scoreInputDialog) {
        scoreInputDialog = new ScoreInputDialog(userManage, username, this);
    }
    scoreInputDialog->show();
}

void StudentWindow::on_logoutButton_clicked()
{
    auto *loginWindow = new LoginWindow(nullptr);
    loginWindow->show();
    this->close();
}