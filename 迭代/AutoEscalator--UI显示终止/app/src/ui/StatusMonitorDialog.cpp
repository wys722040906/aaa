#include "ui/StatusMonitorDialog.h"
#include <QVBoxLayout>
#include <QHeaderView>

StatusMonitorDialog::StatusMonitorDialog(ElevatorDispatcher* dispatcher,
                                       PassengerManager* passengerManager,
                                       QWidget* parent)
    : QDialog(parent), dispatcher(dispatcher), passengerManager(passengerManager) {
    setupUi();
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &StatusMonitorDialog::updateStatus);
    updateTimer->start(500); // 每0.5秒更新一次
}

void StatusMonitorDialog::setupUi() {
    setWindowTitle("电梯运行状态监控");
    
    auto layout = new QVBoxLayout(this);
    
    statusTable = new QTableWidget(this);
    statusTable->setColumnCount(5);
    statusTable->setHorizontalHeaderLabels({
        "电梯编号", "当前楼层", "运行状态", "乘客数量", "等待人数"
    });
    
    statusTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(statusTable);
    
    resize(600, 400);
}

void StatusMonitorDialog::updateStatus() {
    // 实现状态更新逻辑
} 