
#include "studentwindow.h"
#include "ui_studentwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <set>

EvaluationDialog::EvaluationDialog(const QString& studentName, UserManage* userManage, QWidget* parent)
    : QDialog(parent), userManage(userManage) {
    setWindowTitle("评教");
    teacherComboBox = new QComboBox(this);
    evaluationEdit = new QTextEdit(this);
    QPushButton* submitButton = new QPushButton("提交", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(new QLabel("选择教师：", this));
    layout->addWidget(teacherComboBox);
    layout->addWidget(new QLabel("输入评价：", this));
    layout->addWidget(evaluationEdit);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(submitButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    setLayout(layout);

    // 填充已绑定的教师
    auto relationships = userManage->GetRelationshipsForUser(studentName.toStdString());
    std::set<std::string> boundTeachers;
    for (const auto& rel : relationships) {
        if (std::get<0>(rel) == studentName.toStdString()) {
            boundTeachers.insert(std::get<1>(rel));
        }
    }
    for (const auto& teacher : boundTeachers) {
        teacherComboBox->addItem(QString::fromStdString(teacher));
    }
    if (boundTeachers.empty()) {
        teacherComboBox->addItem("无绑定教师");
        teacherComboBox->setEnabled(false);
        submitButton->setEnabled(false);
    }

    connect(submitButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

StudentWindow::StudentWindow(const QString& username, UserManage* userManage, MatchingManager& matchingManager, QWidget* parent)
    : QMainWindow(parent), ui(new Ui::StudentWindow), username(username), userManage(userManage), matchingManager(matchingManager) {
    ui->setupUi(this);
    matchingDialog = new MatchingDialog(userManage, matchingManager, username, this);

    connect(ui->matchTutorButton, &QPushButton::clicked, this, &StudentWindow::on_matchTutorButton_clicked);
    connect(ui->evaluateButton, &QPushButton::clicked, this, &StudentWindow::on_evaluateButton_clicked);
    connect(ui->logoutButton, &QPushButton::clicked, this, &StudentWindow::on_logoutButton_clicked);

    qDebug() << "StudentWindow 初始化完成，用户：" << username;
}

StudentWindow::~StudentWindow() {
    delete matchingDialog;
    delete ui;
}

void StudentWindow::on_matchTutorButton_clicked() {
    matchingDialog->exec();
    qDebug() << "打开 MatchingDialog";
}

void StudentWindow::on_evaluateButton_clicked() {
    EvaluationDialog evalDialog(username, userManage, this);
    if (evalDialog.exec() == QDialog::Accepted) {
        QString teacherName = evalDialog.getTeacherName();
        QString evaluation = evalDialog.getEvaluation().trimmed();
        if (teacherName == "无绑定教师") {
            QMessageBox::warning(this, "错误", "没有可评价的教师");
            return;
        }
        if (evaluation.isEmpty()) {
            QMessageBox::warning(this, "错误", "评价内容不能为空");
            return;
        }
        if (userManage->AddEvaluation(teacherName.toStdString(), evaluation.toStdString())) {
            QMessageBox::information(this, "成功", QString("成功评价教师：%1").arg(teacherName));
        } else {
            QMessageBox::warning(this, "错误", "添加评价失败，教师不存在");
        }
    }
}

void StudentWindow::on_logoutButton_clicked() {
    qDebug() << "用户注销：" << username;
    emit loggedOut();
    close();
}
