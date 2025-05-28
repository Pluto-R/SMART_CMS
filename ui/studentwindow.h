#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include "user_manage.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class StudentWindow; }
QT_END_NAMESPACE

class MatchingDialog;
class ScoreAnalysisDialog;
class ScoreInputDialog;

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    StudentWindow(const QString &username, UserManage *userManage, QWidget *parent = nullptr);
    ~StudentWindow();

private slots:
    void on_matchTeacherButton_clicked();
    void on_analyzeScoresButton_clicked();
    void on_inputScoreButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::StudentWindow *ui;
    QString username;
    UserManage *userManage; // 修改为裸指针
    MatchingDialog *matchingDialog;
    ScoreAnalysisDialog *scoreAnalysisDialog;
    ScoreInputDialog *scoreInputDialog;
};
#endif // STUDENTWINDOW_H