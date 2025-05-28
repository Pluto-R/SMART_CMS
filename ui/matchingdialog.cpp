#include "matchingdialog.h"
#include "ui_matchingdialog.h"
#include <QMessageBox>
#include <QDate>
#include <QTime>

MatchingDialog::MatchingDialog(UserManage *userManage, const QString &studentName, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::MatchingDialog),
    userManage(userManage),
    studentName(studentName)
{
    ui->setupUi(this);
    setWindowTitle("匹配导师");
    setupUi();
}

MatchingDialog::~MatchingDialog()
{
    delete ui;  // 确保正确释放UI资源
}

void MatchingDialog::setupUi()
{
    // 设置学历选项
    ui->educationComboBox->addItem("大学生家教", "0");
    ui->educationComboBox->addItem("在职教师", "1");
    ui->educationComboBox->addItem("特级教师", "2");
    
    // 设置性格选项
    ui->characterComboBox->addItem("温和型", "0");
    ui->characterComboBox->addItem("严格型", "1");
    
    // 设置地区选项
    QStringList districts = {"渝中区", "大渡口区", "江北区", "沙坪坝区", 
                            "九龙坡区", "南岸区", "北碚区", "渝北区", "巴南区"};
    ui->districtComboBox->addItems(districts);
    
    // 设置星期选项
    QStringList weekdays = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    ui->weekdayComboBox->addItems(weekdays);
    
    // 设置时间选择器
    ui->startTimeEdit->setDisplayFormat("HHmm");
    ui->endTimeEdit->setDisplayFormat("HHmm");
}

void MatchingDialog::on_searchButton_clicked()
{
    // 获取匹配条件
    QString education = ui->educationComboBox->currentData().toString();
    QString character = ui->characterComboBox->currentData().toString();
    QString subject = ui->subjectLineEdit->text();
    QString district = ui->districtComboBox->currentText();
    int priceMin = ui->priceMinSpinBox->value();
    int priceMax = ui->priceMaxSpinBox->value();
    QString weekday = ui->weekdayComboBox->currentText();
    
    QTime startTime = ui->startTimeEdit->time();
    QTime endTime = ui->endTimeEdit->time();
    int start = startTime.hour() * 100 + startTime.minute();
    int end = endTime.hour() * 100 + endTime.minute();
    
    if(subject.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入学科");
        return;
    }
    
    if(start >= end) {
        QMessageBox::warning(this, "错误", "结束时间必须大于开始时间");
        return;
    }
    
    // 创建匹配条件
    Matching::MatchingCriteria criteria;
    criteria.education = education.toStdString();
    criteria.character = character.toStdString();
    criteria.subject = subject.toStdString();
    criteria.location = district.toStdString();
    criteria.price_min = priceMin;
    criteria.price_high = priceMax;
    criteria.time_slot = {weekday.toStdString(), {start, end}};
    
    // 执行匹配
    Matching::MatchingManager matchingManager(*userManage);
    auto results = matchingManager.matchTeachers(criteria);
    
    // 显示结果
    ui->teachersListWidget->clear();
    for(const auto& result : results) {
        QString teacherInfo = QString("%1 (评分: %2, 学历: %3, 性格: %4, 价格: %5-%6")
            .arg(QString::fromStdString(result.teacher->GetName()))
            .arg(result.score)
            .arg(QString::fromStdString(result.teacher->education))
            .arg(QString::fromStdString(result.teacher->character))
            .arg(result.teacher->price_min)
            .arg(result.teacher->price_high);
        
        ui->teachersListWidget->addItem(teacherInfo);
    }
}

void MatchingDialog::on_selectButton_clicked()
{
    int currentRow = ui->teachersListWidget->currentRow();
    if(currentRow == -1) {
        QMessageBox::warning(this, "错误", "请先选择一位老师");
        return;
    }
    
    // 获取匹配条件
    QString subject = ui->subjectLineEdit->text();
    QString weekday = ui->weekdayComboBox->currentText();
    
    QTime startTime = ui->startTimeEdit->time();
    QTime endTime = ui->endTimeEdit->time();
    int start = startTime.hour() * 100 + startTime.minute();
    int end = endTime.hour() * 100 + endTime.minute();
    
    QString teacherName = ui->teachersListWidget->currentItem()->text().split(' ').first();
    
    // 绑定师生关系
    std::string timeSlot = QString("%1,%2,%3").arg(weekday).arg(start).arg(end).toStdString();
    userManage->AddRelationship(studentName.toStdString(),
                              teacherName.toStdString(),
                              subject.toStdString(),
                              timeSlot);
    
    QMessageBox::information(this, "成功", "老师选择成功！");
    this->close();
}