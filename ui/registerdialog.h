#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "user_manage.hpp"
#include <QLabel>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterDialog; }
QT_END_NAMESPACE

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    RegisterDialog(UserManage *userManage, QWidget *parent = nullptr);
    ~RegisterDialog();

    void accept() override;

private slots:
    void on_userTypeComboBox_currentIndexChanged(int index);
    void on_addTimeSlotButton_clicked();
    void on_removeTimeSlotButton_clicked();
    void on_registerButton_clicked();

private:
    void setupTeacherFields(bool visible);
    Ui::RegisterDialog *ui;
    UserManage *userManage;
    QLabel *selfDescriptionLabel;
    QTextEdit *selfDescriptionTextEdit;
    bool isProcessing; // Added for click debouncing
};

#endif // REGISTERDIALOG_H