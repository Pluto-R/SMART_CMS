#include "adminwindow.h"
#include "ui_adminwindow.h"
#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

AdminWindow::AdminWindow(const QString &username, UserManage *userManage, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AdminWindow), username(username), userManage(userManage)
{
    ui->setupUi(this);
    setWindowTitle("管理员界面 - " + username);
    setupPendingTeachersTable();
    connect(ui->logoutButton, &QPushButton::clicked, this, &AdminWindow::on_logoutButton_clicked);
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::setupPendingTeachersTable()
{
    ui->pendingTeachersTable->setColumnCount(9);
    ui->pendingTeachersTable->setHorizontalHeaderLabels({
        "用户名", "身份证号", "学历", "性格", "科目", "价格范围", "地区", "可用时间", "操作"
    });
    ui->pendingTeachersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    const auto &pendingTeachers = userManage->GetPendingTeachers();
    ui->pendingTeachersTable->setRowCount(pendingTeachers.size());
    int row = 0;
    for (const auto &pair : pendingTeachers) {
        const auto &teacher = std::get<2>(pair.second);

        QString education = QString::fromStdString(teacher->education);
        QString character = QString::fromStdString(teacher->character);

        QString subjects;
        for (size_t i = 0; i < teacher->subjects.size(); ++i) {
            subjects += QString::fromStdString(teacher->subjects[i]);
            if (i < teacher->subjects.size() - 1) subjects += ", ";
        }

        QString priceRange = QString("%1-%2").arg(teacher->price_min).arg(teacher->price_max);

        QString locations;
        for (size_t i = 0; i < teacher->allow_location.size(); ++i) {
            locations += QString::fromStdString(teacher->allow_location[i]);
            if (i < teacher->allow_location.size() - 1) locations += ", ";
        }

        QString times;
        for (size_t i = 0; i < teacher->available_times.size(); ++i) {
            const auto &time = teacher->available_times[i];
            times += QString("%1 %2-%3")
                        .arg(QString::fromStdString(time.first))
                        .arg(time.second.first)
                        .arg(time.second.second);
            if (i < teacher->available_times.size() - 1) times += "; ";
        }

        ui->pendingTeachersTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(teacher->GetName())));
        ui->pendingTeachersTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(teacher->id_number)));
        ui->pendingTeachersTable->setItem(row, 2, new QTableWidgetItem(education));
        ui->pendingTeachersTable->setItem(row, 3, new QTableWidgetItem(character));
        ui->pendingTeachersTable->setItem(row, 4, new QTableWidgetItem(subjects));
        ui->pendingTeachersTable->setItem(row, 5, new QTableWidgetItem(priceRange));
        ui->pendingTeachersTable->setItem(row, 6, new QTableWidgetItem(locations));
        ui->pendingTeachersTable->setItem(row, 7, new QTableWidgetItem(times));

        QWidget *buttonWidget = new QWidget(this);
        QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
        QPushButton *approveButton = new QPushButton("通过", buttonWidget);
        QPushButton *rejectButton = new QPushButton("拒绝", buttonWidget);
        buttonLayout->addWidget(approveButton);
        buttonLayout->addWidget(rejectButton);
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonWidget->setLayout(buttonLayout);
        ui->pendingTeachersTable->setCellWidget(row, 8, buttonWidget);

        connect(approveButton, &QPushButton::clicked, this, [=]() { on_approveButton_clicked(row); });
        connect(rejectButton, &QPushButton::clicked, this, [=]() { on_rejectButton_clicked(row); });

        ++row;
    }

    qDebug() << "加载了 " << ui->pendingTeachersTable->rowCount() << " 个待审教师";
}

void AdminWindow::on_approveButton_clicked(int row)
{
    QString username = ui->pendingTeachersTable->item(row, 0)->text();
    bool success = userManage->ApproveTeacher(username.toStdString());
    if (success) {
        QMessageBox::information(this, "审批成功", "教师 " + username + " 已通过注册审批。");
        setupPendingTeachersTable();
    } else {
        QMessageBox::warning(this, "审批失败", "无法审批教师 " + username + "，请重试。");
    }
}

void AdminWindow::on_rejectButton_clicked(int row)
{
    QString username = ui->pendingTeachersTable->item(row, 0)->text();
    bool success = userManage->RejectTeacher(username.toStdString());
    if (success) {
        QMessageBox::information(this, "拒绝成功", "教师 " + username + " 的注册申请已被拒绝。");
        setupPendingTeachersTable();
    } else {
        QMessageBox::warning(this, "拒绝失败", "无法拒绝教师 " + username + "，请重试。");
    }
}

void AdminWindow::on_logoutButton_clicked()
{
    this->close();
    emit loggedOut();
}