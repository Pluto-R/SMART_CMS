#ifndef MATCHINGDIALOG_H
#define MATCHINGDIALOG_H
#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QTextEdit>
#include "user_manage.hpp"
#include "matching.hpp"

namespace Ui { class MatchingDialog; }

class ViewEvaluationsDialog : public QDialog {
    Q_OBJECT
public:
    ViewEvaluationsDialog(const QString& teacherName, UserManage* userManage, QWidget* parent = nullptr);
};

class MatchingDialog : public QDialog {
    Q_OBJECT
public:
    MatchingDialog(UserManage* userManage, MatchingManager& matcher, const QString& studentName, QWidget* parent = nullptr);
    ~MatchingDialog();
    MatchingCriteria getMatchingCriteria() const;
private slots:
    void on_searchButton_clicked();
    void on_selectButton_clicked();
    void on_startDynamicMatch();
    void on_pauseDynamicMatch();
    void on_skipToResults();
    void on_viewEvaluationsButton_clicked();
    void processNextTeacher();
private:
    void setupUi();
    void setupDynamicMatchUi();
    void updateTeacherDisplay(const MatchResult& result);
    void showTop5Results();
    Ui::MatchingDialog *ui;
    UserManage *userManage;
    MatchingManager &matcher;
    QString studentName;
    MatchingCriteria currentCriteria;
    std::vector<MatchResult> currentMatchResults;
    std::vector<MatchResult> top5Results;
    size_t currentIndex = 0;
    QTableWidget *dynamicMatchTable;
    QPushButton *startMatchButton;
    QPushButton *pauseMatchButton;
    QPushButton *skipButton;
    QPushButton *viewEvaluationsButton;
    QProgressBar *progressBar;
    QLabel *progressLabel;
    QTimer *matchTimer;
};

#endif