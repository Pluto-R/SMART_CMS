#include "scoreinputdialog.h"
#include "ui_scoreinputdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <set>

ScoreInputDialog::ScoreInputDialog(UserManage* userManage, const QString& studentName, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ScoreInputDialog)
    , userManage(userManage)
    , studentName(studentName)
{
    ui->setupUi(this);
    setWindowTitle("录入成绩");

    // Set score range
    ui->scoreSpinBox->setRange(0, 150);
    ui->scoreSpinBox->setValue(0);
    // Avoid "00" display
    ui->scoreSpinBox->setSpecialValueText(QStringLiteral("0"));

    // Assuming dateEdit exists; if not, update UI
    // ui->dateEdit->setDate(QDate::currentDate());

    setupSubjects();
}

ScoreInputDialog::~ScoreInputDialog()
{
    delete ui;
}

void ScoreInputDialog::setupSubjects()
{
    ui->subjectComboBox->clear();

    std::set<QString> subjects;
    for (const auto& rel : userManage->GetRelationships()) {
        if (std::get<0>(rel) == studentName.toStdString()) {
            QString subject = QString::fromStdString(std::get<2>(rel));
            subjects.insert(subject);
        }
    }

    if (subjects.empty()) {
        ui->subjectComboBox->addItem("无可用科目");
        qDebug() << "No bound subjects for" << studentName;
    } else {
        for (const QString& subject : subjects) {
            ui->subjectComboBox->addItem(subject);
            qDebug() << "Loaded subject:" << subject;
        }
    }
}

void ScoreInputDialog::on_submitButton_clicked()
{
    QString subject = ui->subjectComboBox->currentText();
    int score = ui->scoreSpinBox->value();

    if (subject.isEmpty() || subject == "无可用科目") {
        QMessageBox::warning(this, "错误", "请先选择有效科目");
        return;
    }

    if (score < 0 || score > 150) {
        QMessageBox::warning(this, "错误", "成绩必须在0-150之间");
        return;
    }

    QString dateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    QString teacherName;
    for (const auto& rel : userManage->GetRelationships()) {
        if (std::get<0>(rel) == studentName.toStdString() &&
            std::get<2>(rel) == subject.toStdString()) {
            teacherName = QString::fromStdString(std::get<1>(rel));
            break;
        }
    }

    if (teacherName.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到该科目的绑定老师");
        return;
    }

    try {
        userManage->AddRecord(studentName.toStdString(),
                             teacherName.toStdString(),
                             subject.toStdString(),
                             score,
                             dateStr.toStdString());
        userManage->SaveRecords();

        QMessageBox::information(this, "成功", "成绩录入成功！");
        accept();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("成绩录入失败: %1").arg(e.what()));
    }
}