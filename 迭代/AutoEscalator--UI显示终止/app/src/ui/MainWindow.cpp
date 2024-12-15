#include "ui/MainWindow.h"
#include "ui/ElevatorConfigDialog.h"
#include "ui/StatusMonitorDialog.h"
#include <QToolBar>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    setupElevators();
    createToolbar();
    createStatusBar();
    setupPassengerSimulation();
}

void MainWindow::setupUi() {
    setWindowTitle("电梯管理系统");
    
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto layout = new QHBoxLayout(centralWidget);
    
    // 创建4个电梯显示组件
    for (int i = 0; i < 4; ++i) {
        auto elevatorWidget = new ElevatorWidget(nullptr, this);
        elevatorWidgets.append(elevatorWidget);
        layout->addWidget(elevatorWidget);
    }
}

void MainWindow::createToolbar() {
    auto toolbar = addToolBar("工具栏");
    
    // 配置按钮 - 不使用图标
    auto configAction = toolbar->addAction("电梯配置");
    connect(configAction, &QAction::triggered, this, &MainWindow::showElevatorConfig);
    
    // 紧急模式按钮 - 使用图标
    auto emergencyAction = toolbar->addAction("紧急模式");
    emergencyAction->setCheckable(true);
    connect(emergencyAction, &QAction::toggled, this, &MainWindow::handleEmergencyMode);
    
    // 高峰期模式按钮
    peakHourAction = toolbar->addAction("高峰期模式");
    peakHourAction->setCheckable(true);
    connect(peakHourAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            passengerManager->startPeakHourSimulation();
        } else {
            passengerManager->stopPeakHourSimulation();
        }
    });
    
    // 状态监控按钮
    auto statusAction = toolbar->addAction("状态监控");
    connect(statusAction, &QAction::triggered, this, [this]() {
        auto dialog = new StatusMonitorDialog(dispatcher, passengerManager, this);
        dialog->show();
    });
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage("系统就绪");
    
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &MainWindow::updateStatistics);
    statsTimer->start(1000); // 每秒更新一次统计信息
}

void MainWindow::showElevatorConfig() {
    ElevatorConfigDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto configs = dialog.getConfigurations();
        for (int i = 0; i < configs.size(); ++i) {
            const auto& config = configs[i];
            
            // 确保第一个电梯的配置不变
            if (i == 0) {
                dispatcher->setElevatorSpeed(i, config.speed);
            } else {
                dispatcher->setElevatorRange(i, config.minFloor, config.maxFloor);
                dispatcher->setElevatorSpeed(i, config.speed);
            }
        }
    }
}

void MainWindow::handleEmergencyMode(bool enabled) {
    dispatcher->setEmergencyMode(enabled);
    for (auto widget : elevatorWidgets) {
        if (widget->getElevator()) {
            if (enabled) {
                widget->getElevator()->handleEmergency();
            }
        }
    }
    
    if (enabled) {
        statusBar()->showMessage("紧急模式已启动！", 3000);
        peakHourAction->setEnabled(false);
    } else {
        statusBar()->showMessage("紧急模式已解除", 3000);
        peakHourAction->setEnabled(true);
    }
}

void MainWindow::updateStatistics() {
    auto busyFloors = passengerManager->getBusiestFloors();
    QString statsMsg = QString("最繁忙楼层: %1").arg(
        busyFloors.empty() ? "无" : QString::number(busyFloors[0]));
    statusBar()->showMessage(statsMsg);
}

void MainWindow::setupPassengerSimulation() {
    dispatcher = new ElevatorDispatcher(this);
    passengerManager = new PassengerManager(dispatcher, this);
    
    // 创建并配置电梯
    for (int i = 0; i < elevatorWidgets.size(); ++i) {
        auto elevator = new Elevator(this);
        dispatcher->addElevator(elevator);
        elevatorWidgets[i]->setElevator(elevator);
        
        // 连接故障信号
        connect(elevator, &Elevator::malfunctionOccurred, this, [this, i]() {
            QString msg = QString("电梯 %1 发生故障！").arg(i + 1);
            QMessageBox::warning(this, "故障警告", msg);
        });
    }
}

void MainWindow::setupElevators() {
    // 这个函数可以为空，因为相关功能已经在setupPassengerSimulation中实现
}

void MainWindow::showFloorSettings() {
    // 显示楼层设置对话框
    QDialog dialog(this);
    dialog.setWindowTitle("楼层设置");
    
    auto layout = new QVBoxLayout(&dialog);
    
    // 添加楼层设置控件
    // TODO: 实现具体的楼层设置界面
    
    dialog.exec();
}