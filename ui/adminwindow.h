#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QHeaderView>
#include "user_manage.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class AdminWindow; }
QT_END_NAMESPACE

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    AdminWindow(const QString &username, UserManage *userManage, QWidget *parent = nullptr);
    ~AdminWindow();

signals:
    void loggedOut();

private slots:
    void on_logoutButton_clicked();
    void on_approveButton_clicked(int row);
    void on_rejectButton_clicked(int row);

private:
    void setupPendingTeachersTable();
    Ui::AdminWindow *ui;
    QString username;
    UserManage *userManage;
};

#endif // ADMINWINDOW_H