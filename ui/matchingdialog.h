#ifndef MATCHINGDIALOG_H
#define MATCHINGDIALOG_H

#include <QDialog>
#include "user_manage.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MatchingDialog; }
QT_END_NAMESPACE

class MatchingDialog : public QDialog
{
    Q_OBJECT

public:
    MatchingDialog(UserManage *userManage, const QString &studentName, QWidget *parent = nullptr);
    ~MatchingDialog();

private slots:
    void on_searchButton_clicked();
    void on_selectButton_clicked();

private:
    void setupUi();

    Ui::MatchingDialog *ui;
    UserManage *userManage;
    QString studentName;
};
#endif // MATCHINGDIALOG_H