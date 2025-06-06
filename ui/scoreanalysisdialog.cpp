#include "scoreanalysisdialog.h"
#include "ui_scoreanalysisdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

ScoreAnalysisDialog::ScoreAnalysisDialog(UserManage* userManage, const QString& studentName, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ScoreAnalysisDialog)
    , userManage(userManage)
    , studentName(studentName)
{
    ui->setupUi(this);
    setWindowTitle("成绩分析");
    setupChart();
}

ScoreAnalysisDialog::~ScoreAnalysisDialog()
{
    delete ui;
}

void ScoreAnalysisDialog::setupChart()
{
    auto records = userManage->GetRecords();
    QMap<QString, QtCharts::QLineSeries*> seriesMap;

    for (const auto& record : records) {
        if (std::get<0>(record) == studentName.toStdString()) {
            QString subject = QString::fromStdString(std::get<2>(record));
            int score = std::get<3>(record);
            QString dateStr = QString::fromStdString(std::get<4>(record));
            QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
            qint64 timestamp = QDateTime(date.startOfDay()).toMSecsSinceEpoch(); // Fixed deprecated constructor

            if (!seriesMap.contains(subject)) {
                seriesMap[subject] = new QtCharts::QLineSeries();
                seriesMap[subject]->setName(subject);
            }
            seriesMap[subject]->append(timestamp, score);
        }
    }

    if (seriesMap.isEmpty()) {
        QMessageBox::information(this, "提示", "没有可用的成绩数据");
        return;
    }

    QtCharts::QChart* chart = new QtCharts::QChart();
    chart->setTitle("成绩趋势");
    for (auto* series : seriesMap.values()) {
        chart->addSeries(series);
    }

    QtCharts::QDateTimeAxis* axisX = new QtCharts::QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("日期");
    chart->addAxis(axisX, Qt::AlignBottom);
    for (auto* series : seriesMap.values()) {
        series->attachAxis(axisX);
    }

    QtCharts::QValueAxis* axisY = new QtCharts::QValueAxis();
    axisY->setRange(0, 150);
    axisY->setTitleText("成绩");
    chart->addAxis(axisY, Qt::AlignLeft);
    for (auto* series : seriesMap.values()) {
        series->attachAxis(axisY);
    }

    QtCharts::QChartView* chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout->addWidget(chartView);
}