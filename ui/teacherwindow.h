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
    TeacherWindow(const QString &username, UserManage *userManage, QWidget *parent = nullptr);
    ~TeacherWindow();

private slots:
    void on_analyzeScoresButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::TeacherWindow *ui;
    QString username;
    UserManage *userManage; // 修改为裸指针
    ScoreAnalysisDialog *scoreAnalysisDialog;
};
#endif // TEACHERWINDOW_H