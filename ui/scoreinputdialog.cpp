#include "scoreinputdialog.h"
#include "ui_scoreinputdialog.h"
#include <QMessageBox>
#include <QDate>

ScoreInputDialog::ScoreInputDialog(UserManage *userManage, const QString &studentName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScoreInputDialog)
    , userManage(userManage)
    , studentName(studentName)
{
    ui->setupUi(this);
    setWindowTitle("录入成绩");
    setupSubjects();
}

ScoreInputDialog::~ScoreInputDialog()
{
    delete ui;
}

void ScoreInputDialog::setupSubjects()
{
    // 从师生关系中获取学生已绑定的学科
    ui->subjectComboBox->clear();
    
    for(const auto& rel : userManage->GetRelationships()) {
        if(std::get<0>(rel) == studentName.toStdString()) {
            QString subject = QString::fromStdString(std::get<2>(rel));
            if(ui->subjectComboBox->findText(subject) == -1) {
                ui->subjectComboBox->addItem(subject);
            }
        }
    }
}

void ScoreInputDialog::on_submitButton_clicked()
{
    QString subject = ui->subjectComboBox->currentText();
    int score = ui->scoreSpinBox->value();
    
    if(subject.isEmpty()) {
        QMessageBox::warning(this, "错误", "请选择学科");
        return;
    }
    
    if(score < 0 || score > 100) {
        QMessageBox::warning(this, "错误", "成绩必须在0-100之间");
        return;
    }
    
    // 获取当前日期
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    
    // 查找对应的老师
    QString teacherName;
    for(const auto& rel : userManage->GetRelationships()) {
        if(std::get<0>(rel) == studentName.toStdString() && 
           std::get<2>(rel) == subject.toStdString()) {
            teacherName = QString::fromStdString(std::get<1>(rel));
            break;
        }
    }
    
    if(teacherName.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到该学科的绑定老师");
        return;
    }
    
    // 录入成绩
    userManage->AddRecord(studentName.toStdString(),
                         teacherName.toStdString(),
                         subject.toStdString(),
                         score,
                         date.toStdString());
    userManage->SaveRecords();
    
    QMessageBox::information(this, "成功", "成绩录入成功！");
    this->close();
}