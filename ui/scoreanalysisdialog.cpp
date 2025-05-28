#include "scoreanalysisdialog.h"
#include "ui_scoreanalysisdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <algorithm>
#include <set>

QT_CHARTS_USE_NAMESPACE

ScoreAnalysisDialog::ScoreAnalysisDialog(UserManage* userManage, const QString& studentName, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::ScoreAnalysisDialog),
      userManage(userManage),
      studentName(studentName)
{
    ui->setupUi(this);
    setWindowTitle("成绩分析");

    // 设置学科选项
    ui->subjectComboBox->clear();
    ui->subjectComboBox->addItem("请选择学科", "");
    std::set<std::string> subjects;
    for (const auto& record : userManage->GetRecords()) {
        if (std::get<0>(record) == studentName.toStdString()) {
            subjects.insert(std::get<2>(record));
        }
    }
    for (const auto& subject : subjects) {
        ui->subjectComboBox->addItem(QString::fromStdString(subject), QString::fromStdString(subject));
    }
}

ScoreAnalysisDialog::~ScoreAnalysisDialog()
{
    delete ui;
}

void ScoreAnalysisDialog::on_analyzeButton_clicked()
{
    QString subject = ui->subjectComboBox->currentData().toString();
    if (subject.isEmpty()) {
        QMessageBox::critical(this, "错误", "请选择一个学科");
        return;
    }

    // 提取成绩数据
    std::vector<std::pair<std::string, int>> scores;
    for (const auto& record : userManage->GetRecords()) {
        if (std::get<0>(record) == studentName.toStdString() &&
            std::get<2>(record) == subject.toStdString()) {
            scores.emplace_back(std::get<4>(record), std::get<3>(record));
        }
    }

    if (scores.empty()) {
        QMessageBox::information(this, "提示", "该学科暂无成绩记录");
        return;
    }

    // 按日期排序
    std::sort(scores.begin(), scores.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // 创建折线图
    QLineSeries* series = new QLineSeries();
    for (const auto& score : scores) {
        QDateTime date = QDateTime::fromString(QString::fromStdString(score.first), "yyyy-MM-dd");
        if (date.isValid()) {
            series->append(date.toMSecsSinceEpoch(), score.second);
        }
    }

    if (series->count() == 0) {
        QMessageBox::critical(this, "错误", "无效的日期格式，无法绘制图表");
        delete series;
        return;
    }

    // 创建图表
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(subject + " 成绩趋势");

    // 设置 X 轴（日期）
    QDateTimeAxis* axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("日期");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 设置 Y 轴（成绩，0～150）
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 150);
    axisY->setTitleText("成绩");
    axisY->setLabelFormat("%i");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 设置图表视图
    QChartView* chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout->addWidget(chartView); // 假设 UI 有 chartLayout

    qDebug() << "Generated chart for" << subject << "with" << scores.size() << "records";
}