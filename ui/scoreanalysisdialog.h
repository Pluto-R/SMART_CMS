#ifndef SCOREANALYSISDIALOG_H
#define SCOREANALYSISDIALOG_H

#include "user_manage.hpp"
#include <QDialog>

namespace Ui {
class ScoreAnalysisDialog;
}

class ScoreAnalysisDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScoreAnalysisDialog(UserManage* userManage, const QString& studentName, QWidget* parent = nullptr);
    ~ScoreAnalysisDialog();

private slots:
    void on_analyzeButton_clicked();

private:
    Ui::ScoreAnalysisDialog* ui;
    UserManage* userManage;
    QString studentName;
};

#endif // SCOREANALYSISDIALOG_H