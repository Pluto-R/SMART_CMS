#ifndef TEACHERWINDOW_H
#define TEACHERWINDOW_H

#include <QMainWindow>
#include <memory>
#include "user_manage.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class TeacherWindow; }
QT_END_NAMESPACE

class ScoreAnalysisDialog;

class TeacherWindow : public QMainWindow
{
    Q_OBJECT

public:
    TeacherWindow(const QString &username, std::unique_ptr<UserManage> userManage, QWidget *parent = nullptr);
    ~TeacherWindow();

private slots:
    void on_analyzeScoresButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::TeacherWindow *ui;
    QString username;
    std::unique_ptr<UserManage> userManage;
    ScoreAnalysisDialog *scoreAnalysisDialog;
};
#endif // TEACHERWINDOW_H