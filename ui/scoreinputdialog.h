#ifndef SCOREINPUTDIALOG_H
#define SCOREINPUTDIALOG_H

#include <QDialog>
#include "user_manage.hpp"

namespace Ui {
class ScoreInputDialog;
}

class ScoreInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScoreInputDialog(UserManage *userManage, const QString &studentName, QWidget *parent = nullptr);
    ~ScoreInputDialog();

private slots:
    void on_submitButton_clicked();

private:
    Ui::ScoreInputDialog *ui;
    UserManage *userManage;
    QString studentName;
    
    void setupSubjects();
};

#endif // SCOREINPUTDIALOG_H