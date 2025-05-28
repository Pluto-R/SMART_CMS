#ifndef MATCHINGDIALOG_H
#define MATCHINGDIALOG_H

#include <QDialog>
#include "user_manage.hpp"
#include "matching.hpp"

namespace Ui {
class MatchingDialog;
}

class MatchingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MatchingDialog(UserManage *userManage, const QString &studentName, QWidget *parent = nullptr);
    ~MatchingDialog() override;  // 明确声明为override

private slots:
    void on_searchButton_clicked();
    void on_selectButton_clicked();

private:
    Ui::MatchingDialog *ui;
    UserManage *userManage;
    QString studentName;
    
    void setupUi();
};

#endif // MATCHINGDIALOG_H