#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QApplication>
#include <QTimer>

RegisterDialog::RegisterDialog(UserManage *userManage, QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog), userManage(userManage), isProcessing(false)
{
    qDebug() << "RegisterDialog 构造函数开始";
    if (!userManage) {
        qDebug() << "错误：userManage 为空指针";
        QMessageBox::critical(this, "初始化错误", "用户管理器未初始化！");
        return;
    }
    ui->setupUi(this);
    if (!ui->verticalLayout) {
        qDebug() << "错误：verticalLayout 未定义";
        QMessageBox::critical(this, "初始化错误", "UI布局未正确初始化！");
        return;
    }
    // 动态创建自我描述字段
    selfDescriptionLabel = new QLabel("自我描述:", this);
    selfDescriptionTextEdit = new QTextEdit(this);
    // 添加到主 verticalLayout，在时间段移除按钮和注册按钮之间
    ui->verticalLayout->insertWidget(ui->verticalLayout->indexOf(ui->removeTimeSlotButton),
                                    selfDescriptionLabel);
    ui->verticalLayout->insertWidget(ui->verticalLayout->indexOf(ui->removeTimeSlotButton),
                                    selfDescriptionTextEdit);
    qDebug() << "已将自我描述字段添加到 verticalLayout";
    // 禁用信号以防止初始化时触发
    ui->userTypeComboBox->blockSignals(true);
    // 设置用户类型下拉框
    ui->userTypeComboBox->addItems({"学生", "教师", "管理员"});
    // 设置学历选项
    ui->educationComboBox->addItems({"大学生家教", "专职家教", "在职教师"});
    // 设置性格选项，仅温和型和严格型
    ui->characterComboBox->addItems({"温和型", "严格型"});
    // 设置科目列表
    ui->subjectsListWidget->addItems({"语文", "数学", "英语", "物理", "化学", "生物", "历史", "地理", "政治"});
    ui->subjectsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    // 设置地区列表，重庆主城九区
    ui->locationsListWidget->addItems({
        "渝中区", "大渡口区", "江北区", "沙坪坝区", "九龙坡区",
        "南岸区", "北碚区", "渝北区", "巴南区"
    });
    ui->locationsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    // 设置星期选项
    ui->weekdayComboBox->addItems({"周一", "周二", "周三", "周四", "周五", "周六", "周日"});
    // 恢复信号
    ui->userTypeComboBox->blockSignals(false);
    // 初始化教师字段为隐藏
    setupTeacherFields(false);
    // 连接信号和槽
    connect(ui->userTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegisterDialog::on_userTypeComboBox_currentIndexChanged);
    connect(ui->addTimeSlotButton, &QPushButton::clicked, this, &RegisterDialog::on_addTimeSlotButton_clicked);
    connect(ui->removeTimeSlotButton, &QPushButton::clicked, this, &RegisterDialog::on_removeTimeSlotButton_clicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
    qDebug() << "RegisterDialog 构造函数完成";
}

RegisterDialog::~RegisterDialog()
{
    qDebug() << "RegisterDialog 析构函数";
    delete selfDescriptionLabel;
    delete selfDescriptionTextEdit;
    delete ui;
}

void RegisterDialog::accept()
{
    qDebug() << "RegisterDialog accept 调用";
    QDialog::accept();
}

void RegisterDialog::setupTeacherFields(bool visible)
{
    qDebug() << "setupTeacherFields 调用，visible=" << visible;
    if (!ui->idNumberLabel || !ui->idNumberLineEdit || !selfDescriptionLabel || !selfDescriptionTextEdit) {
        qDebug() << "错误：UI 元素指针为空";
        return;
    }
    ui->idNumberLabel->setVisible(visible);
    ui->idNumberLineEdit->setVisible(visible);
    ui->educationLabel->setVisible(visible);
    ui->educationComboBox->setVisible(visible);
    ui->characterLabel->setVisible(visible);
    ui->characterComboBox->setVisible(visible);
    ui->subjectsLabel->setVisible(visible);
    ui->subjectsListWidget->setVisible(visible);
    ui->priceLabel->setVisible(visible);
    ui->priceMinSpinBox->setVisible(visible);
    ui->toLabel->setVisible(visible);
    ui->priceMaxSpinBox->setVisible(visible);
    ui->locationsLabel->setVisible(visible);
    ui->locationsListWidget->setVisible(visible);
    ui->timeSlotsLabel->setVisible(visible);
    ui->weekdayComboBox->setVisible(visible);
    ui->startTimeEdit->setVisible(visible);
    ui->endTimeEdit->setVisible(visible);
    ui->addTimeSlotButton->setVisible(visible);
    ui->timeSlotsListWidget->setVisible(visible);
    ui->removeTimeSlotButton->setVisible(visible);
    selfDescriptionLabel->setVisible(visible);
    selfDescriptionTextEdit->setVisible(visible);
}

void RegisterDialog::on_userTypeComboBox_currentIndexChanged(int index)
{
    qDebug() << "userTypeComboBox 索引变更：" << index;
    bool isTeacher = ui->userTypeComboBox->currentText() == "教师";
    setupTeacherFields(isTeacher);
}

void RegisterDialog::on_addTimeSlotButton_clicked()
{
    qDebug() << "addTimeSlotButton 点击";
    QString weekday = ui->weekdayComboBox->currentText();
    QTime startTime = ui->startTimeEdit->time();
    QTime endTime = ui->endTimeEdit->time();
    if (startTime >= endTime) {
        qDebug() << "错误：开始时间晚于或等于结束时间";
        QMessageBox::warning(this, "错误", "开始时间必须早于结束时间！");
        return;
    }
    QString timeSlot = QString("%1: %2 - %3")
                       .arg(weekday)
                       .arg(startTime.toString("hh:mm"))
                       .arg(endTime.toString("hh:mm"));
    for (int i = 0; i < ui->timeSlotsListWidget->count(); ++i) {
        if (ui->timeSlotsListWidget->item(i)->text() == timeSlot) {
            qDebug() << "警告：时间段已存在：" << timeSlot;
            QMessageBox::warning(this, "警告", "该时间段已存在！");
            return;
        }
    }
    ui->timeSlotsListWidget->addItem(timeSlot);
    qDebug() << "添加时间段：" << timeSlot;
}

void RegisterDialog::on_removeTimeSlotButton_clicked()
{
    qDebug() << "removeTimeSlotButton 点击";
    QList<QListWidgetItem*> selected = ui->timeSlotsListWidget->selectedItems();
    for (auto item : selected) {
        qDebug() << "移除时间段：" << item->text();
        delete ui->timeSlotsListWidget->takeItem(ui->timeSlotsListWidget->row(item));
    }
}

void RegisterDialog::on_registerButton_clicked()
{
    qDebug() << "registerButton 点击";
    if (isProcessing) {
        qDebug() << "注册处理中，忽略重复点击";
        return;
    }
    isProcessing = true;
    // 断开 registerButton 的 clicked 信号以防止重复点击
    disconnect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
    ui->registerButton->setEnabled(false); // 禁用按钮
    if (!userManage) {
        qDebug() << "错误：userManage 为空指针";
        QMessageBox::critical(this, "错误", "用户管理器未初始化！");
        isProcessing = false;
        connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
        ui->registerButton->setEnabled(true);
        return;
    }

    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString userType = ui->userTypeComboBox->currentText();

    qDebug() << "Register attempt: username=" << username << ", password=" << password << ", userType=" << userType;

    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "错误：用户名或密码为空";
        QMessageBox::warning(this, "注册失败", "请填写用户名和密码！");
        isProcessing = false;
        connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
        ui->registerButton->setEnabled(true);
        return;
    }

    if (userType != "学生" && userType != "教师" && userType != "管理员") {
        qDebug() << "无效的用户类型：" << userType;
        QMessageBox::warning(this, "注册失败", "无效的用户类型！");
        isProcessing = false;
        connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
        ui->registerButton->setEnabled(true);
        return;
    }

    if (userType == "教师") {
        QString idNumber = ui->idNumberLineEdit->text();
        QString education = ui->educationComboBox->currentText();
        QString character = ui->characterComboBox->currentText();
        QList<QListWidgetItem*> selectedSubjects = ui->subjectsListWidget->selectedItems();
        int priceMin = ui->priceMinSpinBox->value();
        int priceMax = ui->priceMaxSpinBox->value();
        QList<QListWidgetItem*> selectedLocations = ui->locationsListWidget->selectedItems();
        int timeSlotsCount = ui->timeSlotsListWidget->count();
        QString selfDescription = selfDescriptionTextEdit->toPlainText().trimmed();

        qDebug() << "Teacher registration:";
        qDebug() << "ID Number:" << idNumber;
        qDebug() << "Education:" << education;
        qDebug() << "Character:" << character;
        qDebug() << "Subjects selected:" << selectedSubjects.count();
        qDebug() << "Price Min:" << priceMin << " Max:" << priceMax;
        qDebug() << "Locations selected:" << selectedLocations.count();
        qDebug() << "Time slots:" << timeSlotsCount;
        qDebug() << "Self description:" << selfDescription;

        if (idNumber.isEmpty() || education.isEmpty() || character.isEmpty() ||
            selectedSubjects.empty() || priceMin <= 0 || priceMax <= priceMin ||
            selectedLocations.empty() || timeSlotsCount == 0 || selfDescription.isEmpty()) {
            qDebug() << "错误：教师信息不完整";
            QMessageBox::warning(this, "注册失败", "请填写完整的教师信息，包括自我描述！");
            isProcessing = false;
            connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
            ui->registerButton->setEnabled(true);
            return;
        }

        std::vector<std::string> subjects;
        for (const auto& item : selectedSubjects) {
            subjects.push_back(item->text().toStdString());
            qDebug() << "添加科目：" << item->text();
        }
        std::vector<std::string> locations;
        for (const auto& item : selectedLocations) {
            locations.push_back(item->text().toStdString());
            qDebug() << "添加地点：" << item->text();
        }
        std::vector<std::pair<std::string, std::pair<int, int>>> availableTimes;
        for (int i = 0; i < ui->timeSlotsListWidget->count(); ++i) {
            QString timeSlot = ui->timeSlotsListWidget->item(i)->text();
            QStringList parts = timeSlot.split(": ");
            if (parts.size() == 2) {
                QStringList times = parts[1].split(" - ");
                if (times.size() == 2) {
                    QTime start = QTime::fromString(times[0], "hh:mm");
                    QTime end = QTime::fromString(times[1], "hh:mm");
                    availableTimes.emplace_back(parts[0].toStdString(),
                                               std::make_pair(start.hour() * 100 + start.minute(),
                                                              end.hour() * 100 + end.minute()));
                    qDebug() << "添加时间段：" << parts[0] << ", " << times[0] << " - " << times[1];
                }
            }
        }

        qDebug() << "调用 RegisterTeacherPending";
        bool result = userManage->RegisterTeacherPending(
            username.toStdString(),
            password.toStdString(),
            idNumber.toStdString(),
            education.toStdString(),
            character.toStdString(),
            subjects,
            locations,
            static_cast<uint16_t>(priceMin),
            static_cast<uint16_t>(priceMax),
            availableTimes,
            selfDescription.toStdString());
        qDebug() << "RegisterTeacherPending 返回：" << result;
        if (result) {
            QMessageBox::information(this, "注册成功", "教师注册已提交，等待管理员审批！");
            // 禁用信号以防止清空时触发 UI 更新
            ui->userTypeComboBox->blockSignals(true);
            // 清空表单
            ui->usernameLineEdit->clear();
            ui->passwordLineEdit->clear();
            ui->idNumberLineEdit->clear();
            ui->subjectsListWidget->clearSelection();
            ui->locationsListWidget->clearSelection();
            ui->timeSlotsListWidget->clear();
            ui->priceMinSpinBox->setValue(0);
            ui->priceMaxSpinBox->setValue(0);
            selfDescriptionTextEdit->clear();
            ui->userTypeComboBox->setCurrentIndex(0);
            ui->userTypeComboBox->blockSignals(false);
            qDebug() << "准备关闭 RegisterDialog";
            // 延迟关闭以确保消息框显示
            QTimer::singleShot(0, this, &RegisterDialog::accept);
            return;
        } else {
            qDebug() << "注册失败，重新启用按钮";
            QMessageBox::warning(this, "注册失败", "该用户名已注册或正在等待审批，请使用其他用户名！");
            isProcessing = false;
            connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
            ui->registerButton->setEnabled(true);
            return;
        }
    } else {
        std::string userTypeCode = (userType == "学生") ? "0" : "2";
        qDebug() << "调用 Register，类型：" << QString::fromStdString(userTypeCode);
        bool result = userManage->Register(userTypeCode, username.toStdString(), password.toStdString());
        qDebug() << "Register 返回：" << result;
        if (result) {
            QMessageBox::information(this, "注册成功", "注册成功！");
            // 禁用信号以防止清空时触发 UI 更新
            ui->userTypeComboBox->blockSignals(true);
            // 清空表单
            ui->usernameLineEdit->clear();
            ui->passwordLineEdit->clear();
            ui->userTypeComboBox->setCurrentIndex(0);
            ui->userTypeComboBox->blockSignals(false);
            qDebug() << "准备关闭 RegisterDialog";
            // 延迟关闭以确保消息框显示
            QTimer::singleShot(0, this, &RegisterDialog::accept);
            return;
        } else {
            qDebug() << "注册失败，重新启用按钮";
            QMessageBox::warning(this, "注册失败", "该用户名已存在，请使用其他用户名！");
            isProcessing = false;
            connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
            ui->registerButton->setEnabled(true);
            return;
        }
    }
}