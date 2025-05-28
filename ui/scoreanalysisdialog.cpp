#include "scoreanalysisdialog.h"
#include "ui_scoreanalysisdialog.h"
#include <QMessageBox>

ScoreAnalysisDialog::ScoreAnalysisDialog(UserManage *userManage, const QString &username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScoreAnalysisDialog),
    userManage(userManage),
    username(username)
{
    ui->setupUi(this);
    setWindowTitle("成绩分析");
    
    isTeacher = (userManage->FindUser(username.toStdString())->GetType() == "1");
    
    sfmlCanvas = new QSFMLCanvas(this);
    ui->verticalLayout->addWidget(sfmlCanvas);
    
    if(isTeacher) {
        ui->subjectLabel->setText("选择学科:");
        ui->subjectLineEdit->setVisible(false);
        ui->subjectComboBox->setVisible(true);
        
        // 使用原始指针避免unique_ptr问题
        Teacher* teacher = userManage->FindTeacherRaw(username.toStdString());
        if(teacher) {
            for(const auto& subject : teacher->subjects) {
                ui->subjectComboBox->addItem(QString::fromStdString(subject));
            }
        }
    } else {
        ui->subjectLabel->setText("输入学科:");
        ui->subjectLineEdit->setVisible(true);
        ui->subjectComboBox->setVisible(false);
    }
}

ScoreAnalysisDialog::~ScoreAnalysisDialog()
{
    delete ui;
    delete sfmlCanvas;
}

void ScoreAnalysisDialog::on_analyzeButton_clicked()
{
    QString subject;
    if(isTeacher) {
        subject = ui->subjectComboBox->currentText();
    } else {
        subject = ui->subjectLineEdit->text();
    }
    
    if(subject.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入学科名称");
        return;
    }
    
    // 明确调用 QWidget 的 setVisible
    sfmlCanvas->QWidget::setVisible(true);
    sfmlCanvas->setFocus();
    
    // 绘制成绩图表
    sfmlCanvas->clear(sf::Color::White);
    
    // 示例：绘制一个简单的坐标系
    sf::VertexArray xAxis(sf::Lines, 2);
    xAxis[0].position = sf::Vector2f(50, 550);
    xAxis[1].position = sf::Vector2f(750, 550);
    xAxis[0].color = xAxis[1].color = sf::Color::Black;
    
    sf::VertexArray yAxis(sf::Lines, 2);
    yAxis[0].position = sf::Vector2f(50, 50);
    yAxis[1].position = sf::Vector2f(50, 550);
    yAxis[0].color = yAxis[1].color = sf::Color::Black;
    
    sfmlCanvas->draw(xAxis);
    sfmlCanvas->draw(yAxis);
    sfmlCanvas->display();
}