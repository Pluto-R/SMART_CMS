#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QComboBox>
#include <QTextEdit>
#include "user_manage.hpp"
#include "matching.hpp"
#include "matchingdialog.h"
namespace Ui { class StudentWindow; }
class EvaluationDialog : public QDialog {
    Q_OBJECT
public:
    EvaluationDialog(const QString& studentName, UserManage* userManage, QWidget* parent = nullptr);
    QString getTeacherName() const { return teacherComboBox->currentText(); }
    QString getEvaluation() const { return evaluationEdit->toPlainText(); }
private:
    QComboBox* teacherComboBox;
    QTextEdit* evaluationEdit;
    UserManage* userManage;
};
class StudentWindow : public QMainWindow {
    Q_OBJECT
public:
    StudentWindow(const QString& username, UserManage* userManage, MatchingManager& matchingManager, QWidget* parent = nullptr);
    ~StudentWindow();
signals:
    void loggedOut();
private slots:
    void on_matchTutorButton_clicked();
    void on_evaluateButton_clicked();
    void on_logoutButton_clicked();
private:
    Ui::StudentWindow *ui;
    QString username;
    UserManage *userManage;
    MatchingManager &matchingManager;
    MatchingDialog *matchingDialog;
};
#endif