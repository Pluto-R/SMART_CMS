#ifndef TEACHERWINDOW_H
#define TEACHERWINDOW_H

#include <QMainWindow>
#include "user_manage.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class TeacherWindow; }
QT_END_NAMESPACE

class TeacherWindow : public QMainWindow
{
    Q_OBJECT

public:
    TeacherWindow(const QString &username, UserManage *userManage, QWidget *parent = nullptr);
    ~TeacherWindow();

signals:
    void loggedOut();

private slots:
    void on_logoutButton_clicked();

private:
    void setupRelationshipsTable();
    Ui::TeacherWindow *ui;
    QString username;
    UserManage *userManage;
};

#endif // TEACHERWINDOW_H