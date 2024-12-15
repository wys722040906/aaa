#include "MainWindow.h"
#include "ElevatorWidget.h"
#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , loginDialog(new LoginDialog(this))
{
    setupUI();
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 电梯显示区域
    QHBoxLayout *elevatorsLayout = new QHBoxLayout();
    for (int i = 0; i < 3; ++i) {
        auto elevatorWidget = new ElevatorWidget(this);
        elevatorWidgets.push_back(elevatorWidget);
        elevatorsLayout->addWidget(elevatorWidget);
    }
    mainLayout->addLayout(elevatorsLayout);
    
    // 控制按钮区域
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    // 楼层选择
    QSpinBox *floorSelect = new QSpinBox(this);
    floorSelect->setRange(1, 20);
    controlLayout->addWidget(new QLabel("目标楼层:"));
    controlLayout->addWidget(floorSelect);
    
    // 紧急按钮
    QPushButton *emergencyBtn = new QPushButton("紧急停止", this);
    emergencyBtn->setStyleSheet("background-color: red; color: white;");
    controlLayout->addWidget(emergencyBtn);
    
    mainLayout->addLayout(controlLayout);
    
    setCentralWidget(centralWidget);
    
    // 显示登录对话框
    loginDialog->exec();
}

void MainWindow::updateElevatorStatus() {
    // TODO: 更新电梯状态显示
} 