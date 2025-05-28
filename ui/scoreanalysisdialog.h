#ifndef SCOREANALYSISDIALOG_H
#define SCOREANALYSISDIALOG_H

#include <QDialog>
#include "user_manage.hpp"
#include "QSFMLCanvas.hpp"

namespace Ui {
class ScoreAnalysisDialog;
}

class ScoreAnalysisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScoreAnalysisDialog(UserManage *userManage, const QString &username, QWidget *parent = nullptr);
    ~ScoreAnalysisDialog();

private slots:
    void on_analyzeButton_clicked();

private:
    Ui::ScoreAnalysisDialog *ui;
    UserManage *userManage;
    QString username;
    bool isTeacher;
    QSFMLCanvas *sfmlCanvas;
};

#endif // SCOREANALYSISDIALOG_H