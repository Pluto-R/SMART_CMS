
#include "matchingdialog.h"
#include "ui_matchingdialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <sstream>
#include <iomanip>

ViewEvaluationsDialog::ViewEvaluationsDialog(const QString& teacherName, UserManage* userManage, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("查看评价：" + teacherName);
    QVBoxLayout* layout = new QVBoxLayout();
    QTextEdit* evalText = new QTextEdit(this);
    evalText->setReadOnly(true);

    auto evaluations = userManage->GetEvaluations(teacherName.toStdString());
    if (evaluations.empty()) {
        evalText->setText("暂无评价");
    } else {
        QString text;
        for (const auto& eval : evaluations) {
            text += QString::fromStdString(eval) + "\n\n";
        }
        evalText->setText(text.trimmed());
    }

    QPushButton* closeButton = new QPushButton("关闭", this);
    layout->addWidget(evalText);
    layout->addWidget(closeButton);
    setLayout(layout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

MatchingDialog::MatchingDialog(UserManage* userManage, MatchingManager& matcher, const QString& studentName, QWidget* parent)
    : QDialog(parent), ui(new Ui::MatchingDialog), userManage(userManage), matcher(matcher), studentName(studentName) {
    qDebug() << "为学生初始化 MatchingDialog：" << studentName;
    ui->setupUi(this);
    setWindowTitle("匹配导师");
    setupUi();
    setupDynamicMatchUi();
    qDebug() << "MatchingDialog 初始化完成";
}

MatchingDialog::~MatchingDialog() {
    qDebug() << "销毁 MatchingDialog，学生：" << studentName;
    currentMatchResults.clear();
    delete ui;
}

MatchingCriteria MatchingDialog::getMatchingCriteria() const {
    return currentCriteria;
}

std::string formatTimeSlot(const std::pair<std::string, std::pair<int, int>>& time_slot) {
    std::stringstream ss;
    ss << time_slot.first << "," 
       << std::setw(4) << std::setfill('0') << time_slot.second.first << "-" 
       << std::setw(4) << std::setfill('0') << time_slot.second.second;
    return ss.str();
}

void MatchingDialog::setupUi() {
    qDebug() << "设置 MatchingDialog UI";

    ui->educationComboBox->clear();
    ui->educationComboBox->addItem("任意", "");
    ui->educationComboBox->addItem("大学生家教", "0");
    ui->educationComboBox->addItem("在职教师", "1");
    ui->educationComboBox->addItem("特级教师", "2");

    ui->characterComboBox->clear();
    ui->characterComboBox->addItem("任意", "");
    ui->characterComboBox->addItem("温和型", "0");
    ui->characterComboBox->addItem("严格型", "1");

    ui->locationComboBox->clear();
    ui->locationComboBox->addItem("任意", "");
    QStringList districts = {"渝中区", "大渡口区", "江北区", "沙坪坝区", "九龙坡区", "南岸区", "北碚区", "渝北区", "巴南区",
                            "长寿区", "綦江区", "潼南区", "铜梁区", "大足区", "荣昌区", "璧山区", "城口县", "丰都县", "垫江县",
                            "忠县", "开州区", "云阳县", "奉节县", "巫山县", "巫溪县", "石柱县", "秀山县", "酉阳县", "彭水县"};
    ui->locationComboBox->addItems(districts);

    ui->weekdayComboBox->clear();
    ui->weekdayComboBox->addItem("任意", "");
    ui->weekdayComboBox->addItem("周一", "Mon");
    ui->weekdayComboBox->addItem("周二", "Tue");
    ui->weekdayComboBox->addItem("周三", "Wed");
    ui->weekdayComboBox->addItem("周四", "Thu");
    ui->weekdayComboBox->addItem("周五", "Fri");
    ui->weekdayComboBox->addItem("周六", "Sat");
    ui->weekdayComboBox->addItem("周日", "Sun");

    ui->startTimeEdit->setTime(QTime(0, 0));
    ui->endTimeEdit->setTime(QTime(23, 59));

    ui->priceMinSpinBox->setMinimum(0);
    ui->priceMaxSpinBox->setMinimum(0);
    ui->priceMinSpinBox->setMaximum(10000);
    ui->priceMaxSpinBox->setMaximum(10000);
    ui->priceMinSpinBox->setValue(0);
    ui->priceMaxSpinBox->setValue(10000);

    connect(ui->searchButton, &QPushButton::clicked, this, &MatchingDialog::on_searchButton_clicked);
    connect(ui->selectButton, &QPushButton::clicked, this, &MatchingDialog::on_selectButton_clicked);
}

void MatchingDialog::setupDynamicMatchUi() {
    dynamicMatchTable = new QTableWidget(this);
    dynamicMatchTable->setColumnCount(4);
    dynamicMatchTable->setHorizontalHeaderLabels({"姓名", "科目", "评分", "时间段"});
    dynamicMatchTable->setSelectionMode(QAbstractItemView::SingleSelection);
    dynamicMatchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    startMatchButton = new QPushButton("开始动态匹配", this);
    pauseMatchButton = new QPushButton("暂停", this);
    skipButton = new QPushButton("跳到结果", this);
    viewEvaluationsButton = new QPushButton("查看老师评价", this);

    progressBar = new QProgressBar(this);
    progressLabel = new QLabel("匹配进度：0%", this);
    matchTimer = new QTimer(this);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(dynamicMatchTable);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(startMatchButton);
    buttonLayout->addWidget(pauseMatchButton);
    buttonLayout->addWidget(skipButton);
    buttonLayout->addWidget(viewEvaluationsButton);
    layout->addLayout(buttonLayout);
    layout->addWidget(progressBar);
    layout->addWidget(progressLabel);
    ui->verticalLayout->addLayout(layout);

    connect(startMatchButton, &QPushButton::clicked, this, &MatchingDialog::on_startDynamicMatch);
    connect(pauseMatchButton, &QPushButton::clicked, this, &MatchingDialog::on_pauseDynamicMatch);
    connect(skipButton, &QPushButton::clicked, this, &MatchingDialog::on_skipToResults);
    connect(viewEvaluationsButton, &QPushButton::clicked, this, &MatchingDialog::on_viewEvaluationsButton_clicked);
    connect(matchTimer, &QTimer::timeout, this, &MatchingDialog::processNextTeacher);
}

void MatchingDialog::on_searchButton_clicked() {
    currentCriteria.subject = ui->subjectLineEdit->text().toStdString();
    currentCriteria.education = ui->educationComboBox->currentData().toString().toStdString();
    currentCriteria.character = ui->characterComboBox->currentData().toString().toStdString();
    currentCriteria.location = ui->locationComboBox->currentData().toString().toStdString();
    currentCriteria.price_min = ui->priceMinSpinBox->value();
    currentCriteria.price_max = ui->priceMaxSpinBox->value();
    
    std::string weekday = ui->weekdayComboBox->currentData().toString().toStdString();
    int start_time = ui->startTimeEdit->time().hour() * 100 + ui->startTimeEdit->time().minute();
    int end_time = ui->endTimeEdit->time().hour() * 100 + ui->endTimeEdit->time().minute();
    currentCriteria.time_slot = {weekday, {start_time, end_time}};

    qDebug() << "Search criteria: Subject=" << QString::fromStdString(currentCriteria.subject)
             << ", Education=" << QString::fromStdString(currentCriteria.education)
             << ", Character=" << QString::fromStdString(currentCriteria.character)
             << ", Location=" << QString::fromStdString(currentCriteria.location)
             << ", Price=" << currentCriteria.price_min << "-" << currentCriteria.price_max
             << ", Time=" << QString::fromStdString(currentCriteria.time_slot.first)
             << "," << currentCriteria.time_slot.second.first << "-" << currentCriteria.time_slot.second.second;

    currentMatchResults = matcher.matchTeachers(currentCriteria);
    currentIndex = 0;
    top5Results.clear();
    dynamicMatchTable->setRowCount(0);

    if (currentMatchResults.empty()) {
        QMessageBox::information(this, "匹配结果", "没有找到符合条件的教师");
    } else {
        startMatchButton->setEnabled(true);
        pauseMatchButton->setEnabled(false);
        skipButton->setEnabled(true);
    }
}

void MatchingDialog::on_selectButton_clicked() {
    int row = dynamicMatchTable->currentRow();
    if (row < 0 || static_cast<size_t>(row) >= top5Results.size()) {
        QMessageBox::warning(this, "错误", "请先选择一名教师");
        return;
    }

    auto& result = top5Results[row];
    userManage->AddRelationship(studentName.toStdString(), result.teacher->GetName(),
                                currentCriteria.subject, formatTimeSlot(result.matched_time_slot));
    QMessageBox::information(this, "成功", QString("已选择教师：%1").arg(QString::fromStdString(result.teacher->GetName())));
    accept();
}

void MatchingDialog::on_startDynamicMatch() {
    if (currentMatchResults.empty()) {
        QMessageBox::warning(this, "错误", "没有可匹配的教师");
        return;
    }
    currentIndex = 0;
    dynamicMatchTable->setRowCount(0);
    top5Results.clear();
    matchTimer->start(1000);
    startMatchButton->setEnabled(false);
    pauseMatchButton->setEnabled(true);
    skipButton->setEnabled(true);
}

void MatchingDialog::on_pauseDynamicMatch() {
    matchTimer->stop();
    startMatchButton->setEnabled(true);
    pauseMatchButton->setEnabled(false);
}

void MatchingDialog::on_skipToResults() {
    matchTimer->stop();
    showTop5Results();
    startMatchButton->setEnabled(true);
    pauseMatchButton->setEnabled(false);
    skipButton->setEnabled(false);
}

void MatchingDialog::on_viewEvaluationsButton_clicked() {
    int row = dynamicMatchTable->currentRow();
    if (row < 0 || static_cast<size_t>(row) >= top5Results.size()) {
        QMessageBox::warning(this, "错误", "请先选择一名教师");
        return;
    }
    QString teacherName = QString::fromStdString(top5Results[row].teacher->GetName());
    ViewEvaluationsDialog dialog(teacherName, userManage, this);
    dialog.exec();
}

void MatchingDialog::processNextTeacher() {
    if (currentIndex >= currentMatchResults.size()) {
        matchTimer->stop();
        showTop5Results();
        startMatchButton->setEnabled(true);
        pauseMatchButton->setEnabled(false);
        skipButton->setEnabled(false);
        return;
    }

    updateTeacherDisplay(currentMatchResults[currentIndex]);
    if (top5Results.size() < 5) {
        top5Results.push_back(currentMatchResults[currentIndex]);
    } else if (currentMatchResults[currentIndex].score > top5Results.back().score) {
        top5Results.back() = currentMatchResults[currentIndex];
        std::sort(top5Results.begin(), top5Results.end(),
                  [](const MatchResult& a, const MatchResult& b) { return a.score > b.score; });
    }

    currentIndex++;
    int progress = static_cast<int>((currentIndex * 100) / currentMatchResults.size());
    progressBar->setValue(progress);
    progressLabel->setText(QString("匹配进度：%1%").arg(progress));
}

void MatchingDialog::updateTeacherDisplay(const MatchResult& result) {
    int row = dynamicMatchTable->rowCount();
    dynamicMatchTable->insertRow(row);
    dynamicMatchTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(result.teacher->GetName())));
    dynamicMatchTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(currentCriteria.subject)));
    dynamicMatchTable->setItem(row, 2, new QTableWidgetItem(QString::number(result.score)));
    dynamicMatchTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(formatTimeSlot(result.matched_time_slot))));
}

void MatchingDialog::showTop5Results() {
    dynamicMatchTable->setRowCount(0);
    for (const auto& result : top5Results) {
        updateTeacherDisplay(result);
    }
    progressBar->setValue(100);
    progressLabel->setText("匹配进度：100%");
}
