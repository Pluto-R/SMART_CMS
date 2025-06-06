#ifndef SCOREANALYSISDIALOG_H
#define SCOREANALYSISDIALOG_H

#include <QDialog>
#include "user_manage.hpp"

namespace Ui {
class ScoreAnalysisDialog;
}

class ScoreAnalysisDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScoreAnalysisDialog(UserManage* userManage, const QString& studentName, QWidget* parent = nullptr);
    ~ScoreAnalysisDialog();

private:
    void setupChart(); // Added declaration
    Ui::ScoreAnalysisDialog* ui;
    UserManage* userManage;
    QString studentName;
};

#endif // SCOREANALYSISDIALOG_H