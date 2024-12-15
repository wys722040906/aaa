#include "mainwindow.h"
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QRandomGenerator>
#include <QDebug>
#include "../statistics/statisticsdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(new QWidget(this))
    , m_mainLayout(new QHBoxLayout(m_centralWidget))
    , m_controlLayout(new QVBoxLayout())
    , m_floorDisplayArea(new QWidget(this))
    , m_elevatorDisplayArea(new QWidget(this))
    , m_startButton(new QPushButton("开始", this))
    , m_resetButton(new QPushButton("重置", this))
    , m_rushHourLabel(new QLabel(this))
    , m_settingsDialog(nullptr)
    , m_statisticsDialog(nullptr)
    , m_isRunning(false)
    , m_simulationTime(0)
    , m_simulationTimer(new QTimer(this))
    , m_passengerGenerator(new QTimer(this))
    , m_dayDuration(240)        // 改为240秒 (24的倍数，每10秒表示1小时)
    , m_floorTravelTime(5)      // 默认5秒/层
    , m_maxPassengers(12)       // 默认最大12人
    , m_idleTime(10)           // 默认空闲10秒
    , m_requestInterval(10)     // 默认10秒生成随机请求
{
    setupUI();
    setupMenuBar();
    setupControlPanel();
    setupFloorDisplay();
    setupElevatorDisplay();  // 先创建电梯
    
    // 创建电梯控制器（在电梯创建之后）
    m_elevatorController = new ElevatorController(m_elevatorWidgets, this);
    
    setupConnections();
    
    setCentralWidget(m_centralWidget);
    setWindowTitle("电梯管理系统");
}

MainWindow::~MainWindow() {
    // 清理资源
}

void MainWindow::setupUI() {
    // 设置窗口大小为屏幕的约53%（80% * 2/3 ≈ 53%）
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.53;
    int height = screenGeometry.height() * 0.53;
    resize(width, height);

    // 调整控制面板宽度
    m_controlLayout->setSpacing(10);
    m_controlLayout->setContentsMargins(10, 10, 10, 10);

    // 调整主布局间距
    m_mainLayout->setSpacing(13);  // 原来是20，调整为13
    m_mainLayout->setContentsMargins(13, 13, 13, 13);  // 原来是20，调整为13
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* settingsMenu = menuBar->addMenu("设置");
    QAction* settingsAction = settingsMenu->addAction("系统设置");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsTriggered);

    QMenu* userMenu = menuBar->addMenu("用户");
    QAction* logoutAction = userMenu->addAction("注销");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogoutTriggered);
}

void MainWindow::setupControlPanel() {
    // 控制面板布局
    QWidget* controlPanel = new QWidget(this);
    controlPanel->setLayout(m_controlLayout);
    controlPanel->setFixedWidth(100);  // 原来是150，调整为100

    // 添加控制按钮
    m_startButton->setFixedHeight(40);
    m_resetButton->setFixedHeight(40);
    
    m_controlLayout->addWidget(m_startButton);
    m_controlLayout->addWidget(m_resetButton);
    m_controlLayout->addWidget(m_rushHourLabel);
    m_controlLayout->addStretch();

    // 添加到主布局
    m_mainLayout->addWidget(controlPanel);
}

void MainWindow::setupConnections() {
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    
    // 设置模拟计时器
    connect(m_simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    connect(m_passengerGenerator, &QTimer::timeout, this, &MainWindow::checkRushHour);
    connect(m_simulationTimer, &QTimer::timeout, m_elevatorController, &ElevatorController::update);
    connect(m_elevatorController, &ElevatorController::statisticsUpdated,
        [this](int floor, int count) {
            if (m_statisticsDialog) {
                QMap<int, int> stats;
                stats[floor] = count;
                m_statisticsDialog->updateStatistics(stats);
            }
        });
    
    // 添加楼层请求完成的信号连接
    connect(m_elevatorController, &ElevatorController::floorRequestCompleted,
        [this](int floor, int targetFloor, int count) {
            // 更新起始楼层和目标楼层的状态
            for (auto floorWidget : m_floorWidgets) {
                if (floorWidget->getFloorNumber() == floor) {
                    floorWidget->updatePassengerRequest(targetFloor, -count);
                }
            }
        });
}

void MainWindow::onStartClicked() {
    if (!m_isRunning) {
        m_isRunning = true;
        m_startButton->setText("运行中...");
        m_startButton->setEnabled(false);
        m_simulationTimer->start(1000);  // 每秒更新一次
        m_passengerGenerator->start(m_requestInterval * 1000);
        m_elevatorController->setParameters(m_maxPassengers, m_floorTravelTime, m_idleTime);
    }
}

void MainWindow::onResetClicked() {
    m_isRunning = false;
    m_simulationTime = 0;
    m_simulationTimer->stop();
    m_passengerGenerator->stop();
    m_startButton->setText("开始");
    m_startButton->setEnabled(true);
    m_rushHourLabel->clear();
    
    // 重置电梯状态但保持参数设置
    m_elevatorController->reset();
    
    // 重新设置参数，确保保持用户的设置
    m_elevatorController->setParameters(m_maxPassengers, m_floorTravelTime, m_idleTime);
}

void MainWindow::onSettingsTriggered() {
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(this);
        
        // 连接设置变更信号
        connect(m_settingsDialog, &SettingsDialog::settingsChanged,
            [this](int maxPassengers, int floorTravelTime, 
                   int idleTime, int dayDuration, int requestInterval) {
                // 更新系统参数
                m_maxPassengers = maxPassengers;
                m_floorTravelTime = floorTravelTime;
                m_idleTime = idleTime;
                m_dayDuration = dayDuration;
                m_requestInterval = requestInterval;
                
                // 如果系统正在运行，需要重新启动
                if (m_isRunning) {
                    onResetClicked();
                    onStartClicked();
                }
            });
    }
    
    // 设置当前值
    m_settingsDialog->setMaxPassengers(m_maxPassengers);
    m_settingsDialog->setFloorTravelTime(m_floorTravelTime);
    m_settingsDialog->setIdleTime(m_idleTime);
    m_settingsDialog->setDayDuration(m_dayDuration);
    m_settingsDialog->setRequestInterval(m_requestInterval);
    
    m_settingsDialog->exec();
}

void MainWindow::onLogoutTriggered() {
    // 返回登录界面
    this->close();
    emit QApplication::instance()->quit();  // 重新启动应用
}

void MainWindow::updateSimulation() {
    if (!m_isRunning) return;
    
    m_simulationTime++;
    
    // 检查是否到达一天结束
    if (m_simulationTime >= m_dayDuration) {
        // 保存当前参数
        int currentMaxPassengers = m_maxPassengers;
        int currentFloorTravelTime = m_floorTravelTime;
        int currentIdleTime = m_idleTime;
        int currentDayDuration = m_dayDuration;
        int currentRequestInterval = m_requestInterval;
        
        onResetClicked();
        
        // 恢复保存的参数
        m_maxPassengers = currentMaxPassengers;
        m_floorTravelTime = currentFloorTravelTime;
        m_idleTime = currentIdleTime;
        m_dayDuration = currentDayDuration;
        m_requestInterval = currentRequestInterval;
        
        // 重新设置参数
        m_elevatorController->setParameters(m_maxPassengers, m_floorTravelTime, m_idleTime);
        
        // 显示统计图表
        if (!m_statisticsDialog) {
            m_statisticsDialog = new StatisticsDialog(this);
        }
        m_statisticsDialog->exec();
        m_statisticsDialog->clearStatistics();
    }
}

void MainWindow::checkRushHour() {
    if (!m_isRunning) return;
    
    // 计算当前时间点
    double timeRatio = static_cast<double>(m_simulationTime) / m_dayDuration;
    
    // 上班高峰 (6/24)
    if (qAbs(timeRatio - 6.0/24.0) < 0.01) {
        createRushHourPassengers(true, "上班");
        m_rushHourLabel->setText("上班高峰");
    }
    // 午餐高峰 (11/24)
    else if (qAbs(timeRatio - 11.0/24.0) < 0.01) {
        createRushHourPassengers(false, "午餐");
        m_rushHourLabel->setText("午餐高峰");
    }
    // 下午上班高峰 (14/24)
    else if (qAbs(timeRatio - 14.0/24.0) < 0.01) {
        createRushHourPassengers(true, "上班");
        m_rushHourLabel->setText("上班高峰");
    }
    // 下班高峰 (16/24)
    else if (qAbs(timeRatio - 16.0/24.0) < 0.01) {
        createRushHourPassengers(false, "下班");
        m_rushHourLabel->setText("下班高峰");
    }
}

void MainWindow::createRushHourPassengers(bool isGroundFloor, const QString& type) {
    m_elevatorController->addRushHourRequests(isGroundFloor, type);
}

void MainWindow::setupFloorDisplay() {
    // 创建楼层显示区域的布局
    auto floorLayout = new QVBoxLayout(m_floorDisplayArea);
    floorLayout->setSpacing(5);
    floorLayout->setContentsMargins(10, 10, 10, 10);

    // 从上到下创建14层楼的显示（14楼到1楼）
    for (int i = 14; i >= 1; --i) {
        auto floorWidget = new FloorWidget(i, this);
        
        // 连接乘客请求信号
        connect(floorWidget, &FloorWidget::requestAdded, 
            [this](int currentFloor, int targetFloor, int count) {
                m_elevatorController->addRequest(currentFloor, targetFloor, count);
            });
        
        m_floorWidgets.push_back(floorWidget);
        floorLayout->addWidget(floorWidget);
    }

    // 设置楼层显示区域的样式
    m_floorDisplayArea->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 5px;
        }
    )");

    // 设置固定宽度，但高度可以自适应
    m_floorDisplayArea->setFixedWidth(200);  // 原来是300，调整为200
    
    // 添加到主布局
    m_mainLayout->addWidget(m_floorDisplayArea);
}

void MainWindow::setupElevatorDisplay() {
    // 创建电梯显示区域的布局
    auto elevatorLayout = new QHBoxLayout(m_elevatorDisplayArea);
    elevatorLayout->setSpacing(10);
    elevatorLayout->setContentsMargins(10, 10, 10, 10);

    // 创建4部电梯
    for (int i = 0; i < 4; ++i) {
        auto elevatorWidget = new ElevatorWidget(i + 1, this);
        m_elevatorWidgets.push_back(elevatorWidget);  // 先添加到数组中
        elevatorLayout->addWidget(elevatorWidget);
        
        // 连接电梯状态变化信号
        connect(elevatorWidget, &ElevatorWidget::floorChanged,
            [this](int elevatorId, int floor) {
                qDebug() << "Elevator" << elevatorId << "moved to floor" << floor;
            });
        
        connect(elevatorWidget, &ElevatorWidget::directionChanged,
            [this](int elevatorId, Direction direction) {
                qDebug() << "Elevator" << elevatorId << "changed direction";
            });
        
        connect(elevatorWidget, &ElevatorWidget::passengerCountChanged,
            [this](int elevatorId, int count) {
                qDebug() << "Elevator" << elevatorId << "passenger count:" << count;
            });
    }

    // 设置电梯显示区域的样式
    m_elevatorDisplayArea->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 5px;
        }
    )");

    // 添加到主布局
    m_mainLayout->addWidget(m_elevatorDisplayArea);
} 