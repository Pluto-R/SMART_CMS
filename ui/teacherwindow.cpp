
#include "teacherwindow.h"
#include "ui_teacherwindow.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDebug>

TeacherWindow::TeacherWindow(const QString &username, UserManage *userManage, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::TeacherWindow), username(username), userManage(userManage)
{
    ui->setupUi(this);
    setWindowTitle("教师界面 - " + username);
    setupRelationshipsTable();
    connect(ui->logoutButton, &QPushButton::clicked, this, &TeacherWindow::on_logoutButton_clicked);
}

TeacherWindow::~TeacherWindow()
{
    delete ui;
}

void TeacherWindow::setupRelationshipsTable()
{
    QTableWidget *table = new QTableWidget(this);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"学生姓名", "科目", "时间段"});
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto relationships = userManage->GetRelationshipsForUser(username.toStdString());
    table->setRowCount(relationships.size());
    for (size_t i = 0; i < relationships.size(); ++i) {
        const auto &rel = relationships[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(std::get<0>(rel))));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(std::get<2>(rel))));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(std::get<3>(rel))));
    }

    QVBoxLayout *layout = new QVBoxLayout(ui->centralwidget);
    layout->addWidget(table);
    ui->centralwidget->setLayout(layout);
}

void TeacherWindow::on_logoutButton_clicked()
{
    this->close();
    emit loggedOut();
}
