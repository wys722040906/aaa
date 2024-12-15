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
    , m_dayDuration(240)
    , m_floorTravelTime(5)
    , m_maxPassengers(12)
    , m_idleTime(10)
    , m_requestInterval(10)
    , m_randomPassengerCount(3)
    , m_peakPassengerCount(15)
    , m_timeLabel(new QLabel(this))
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
    controlPanel->setFixedWidth(120);  // 稍微加宽以适应时间显示

    // 创建时间显示区域
    auto timeWidget = new QWidget(this);
    auto timeLayout = new QVBoxLayout(timeWidget);
    timeLayout->setSpacing(2);
    timeLayout->setContentsMargins(0, 0, 0, 0);

    // 添加时间显示标签
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet(R"(
        QLabel {
            font-size: 16px;
            font-weight: bold;
            color: #333;
            padding: 5px;
            border: 1px solid #ccc;
            border-radius: 3px;
            background: #f5f5f5;
            min-height: 45px;
        }
    )");
    timeLayout->addWidget(m_timeLabel);

    // 设置高峰期标签样式
    m_rushHourLabel->setAlignment(Qt::AlignCenter);
    m_rushHourLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            font-weight: bold;
            color: #e74c3c;
            padding: 3px;
            min-height: 20px;
        }
    )");
    timeLayout->addWidget(m_rushHourLabel);

    // 添加时间显示区域到控制面板
    m_controlLayout->addWidget(timeWidget);
    
    // 添加一些间距
    m_controlLayout->addSpacing(15);
    
    // 添加控制按钮
    m_startButton->setFixedHeight(40);
    m_startButton->setStyleSheet(R"(
        QPushButton {
            font-size: 14px;
            font-weight: bold;
            border-radius: 5px;
            background-color: #2ecc71;
            color: white;
        }
        QPushButton:hover {
            background-color: #27ae60;
        }
        QPushButton:disabled {
            background-color: #95a5a6;
        }
    )");
    
    m_resetButton->setFixedHeight(40);
    m_resetButton->setStyleSheet(R"(
        QPushButton {
            font-size: 14px;
            font-weight: bold;
            border-radius: 5px;
            background-color: #e74c3c;
            color: white;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
    )");
    
    m_controlLayout->addWidget(m_startButton);
    m_controlLayout->addWidget(m_resetButton);
    m_controlLayout->addStretch();

    // 添加到主布局
    m_mainLayout->addWidget(controlPanel);
}

void MainWindow::setupConnections() {
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    
    // 设置模拟计时器，只使用一个定时器
    connect(m_simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    connect(m_simulationTimer, &QTimer::timeout, this, &MainWindow::checkRushHour);
    connect(m_simulationTimer, &QTimer::timeout, m_elevatorController, &ElevatorController::update);
    
    // 移除 m_passengerGenerator 相关的连接
    // connect(m_passengerGenerator, &QTimer::timeout, this, &MainWindow::checkRushHour);
    
    // 统计信号连接
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

    // 连接楼层请求信号
    for (int i = 1; i <= 14; ++i) {
        FloorWidget* floor = m_floorWidgets[i - 1];
        
        // 连接控制器的信号到楼层控件
        connect(m_elevatorController, &ElevatorController::floorRequestAdded,
                floor, &FloorWidget::onRequestAdded);
        
        connect(m_elevatorController, &ElevatorController::floorRequestCompleted,
                floor, &FloorWidget::updatePassengerRequest);
        
        // 连接楼层的请求信号到控制器
        connect(floor, &FloorWidget::requestAdded,
                [this](int currentFloor, int targetFloor, int count) {
                    m_elevatorController->addRequest(currentFloor, targetFloor, count);
                });
    }
}

void MainWindow::onStartClicked() {
    if (!m_isRunning) {
        m_isRunning = true;
        m_startButton->setText("运行中...");
        m_startButton->setEnabled(false);
        m_simulationTimer->start(1000);  // 每秒更新一次
        
        // 移除独立的乘客生成定时器
        // m_passengerGenerator->start(m_requestInterval * 1000);
        
        m_elevatorController->setParameters(m_maxPassengers, m_floorTravelTime, m_idleTime);
    }
}

void MainWindow::onResetClicked() {
    // 停止计时器
    m_simulationTimer->stop();
    
    // 重置电梯控制器
    m_elevatorController->reset();
    
    // 重置所有楼层状态
    for (auto floorWidget : m_floorWidgets) {
        floorWidget->clearPassengerRequests();
    }
    
    // 重置时间和标签
    m_simulationTime = 0;
    m_timeLabel->setText("00:00");
    m_rushHourLabel->clear();
    
    // 重置开始按钮状态
    m_startButton->setText("开始");
    m_startButton->setEnabled(true);
    m_isRunning = false;
    
    // 重新设置参数
    m_elevatorController->setParameters(m_maxPassengers, m_floorTravelTime, m_idleTime);
}

void MainWindow::onSettingsTriggered() {
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(this);
        
        // 连接设置变更信号
        connect(m_settingsDialog, &SettingsDialog::settingsChanged,
            [this](int maxPassengers, int floorTravelTime, 
                   int idleTime, int dayDuration, int requestInterval,
                   int randomPassengerCount, int peakPassengerCount,
                   int doorTime) {  // 添加开关门时间参数
                // 更新系统参数
                m_maxPassengers = maxPassengers;
                m_floorTravelTime = floorTravelTime;
                m_idleTime = idleTime;
                m_dayDuration = dayDuration;
                m_requestInterval = requestInterval;
                m_randomPassengerCount = randomPassengerCount;
                m_peakPassengerCount = peakPassengerCount;
                
                // 更新电梯控制器参数
                if (m_elevatorController) {
                    m_elevatorController->setParameters(m_maxPassengers, m_floorTravelTime, m_idleTime);
                    m_elevatorController->setRandomPassengerCount(m_randomPassengerCount);
                    m_elevatorController->setPeakPassengerCount(m_peakPassengerCount);
                    m_elevatorController->setDoorTime(doorTime);  // 设置开关门时间
                }
            });
    }
    
    // 设置当前值
    m_settingsDialog->setMaxPassengers(m_maxPassengers);
    m_settingsDialog->setFloorTravelTime(m_floorTravelTime);
    m_settingsDialog->setIdleTime(m_idleTime);
    m_settingsDialog->setDayDuration(m_dayDuration);
    m_settingsDialog->setRequestInterval(m_requestInterval);
    m_settingsDialog->setRandomPassengerCount(m_randomPassengerCount);
    m_settingsDialog->setPeakPassengerCount(m_peakPassengerCount);
    
    // 显示设置对话框
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
    
    // 更新时间显示
    m_timeLabel->setText(formatSimulationTime(m_simulationTime));
    
    // 检查高峰期和生成请求
    checkRushHour();
    
    // 检查是否到达一天结束
    if (m_simulationTime >= m_dayDuration) {
        // 显示统计图表
        if (!m_statisticsDialog) {
            m_statisticsDialog = new StatisticsDialog(this);
        }
        m_statisticsDialog->exec();
        
        // 重置系统
        onResetClicked();
        
        // 清空统计数据
        m_statisticsDialog->clearStatistics();
    }
}

void MainWindow::checkRushHour() {
    if (!m_isRunning) return;
    
    // 计算每小时对应的秒数
    int secondsPerHour = m_dayDuration / 24;
    
    // 计算各个时间点对应的秒数
    int time_8am = 8 * secondsPerHour;
    int time_11am = 11 * secondsPerHour;
    int time_2pm = 14 * secondsPerHour;
    int time_5pm = 17 * secondsPerHour;
    
    // 更新高峰时段标签和生成乘客
    if (m_simulationTime == time_8am) {  // 8:00
        createRushHourPassengers(true, "上班");
        m_rushHourLabel->setText("清晨上班高峰");
    }
    else if (m_simulationTime == time_11am) {  // 11:00
        createRushHourPassengers(false, "午餐");
        m_rushHourLabel->setText("晌午下班高峰");
    }
    else if (m_simulationTime == time_2pm) {  // 14:00
        createRushHourPassengers(true, "上班");
        m_rushHourLabel->setText("下午上班高峰");
    }
    else if (m_simulationTime == time_5pm) {  // 17:00
        createRushHourPassengers(false, "下班");
        m_rushHourLabel->setText("落日下班高峰");
    }
    else if (m_simulationTime % m_requestInterval == 0) {  // 只在定间隔时生成随机请求
        // 非高峰时段，生成随机乘客
        m_elevatorController->generateRandomPassengers();
        
        // 在高峰期结束后清除标签
        if (m_simulationTime == time_8am + secondsPerHour ||  // 9:00
            m_simulationTime == time_11am + secondsPerHour || // 12:00
            m_simulationTime == time_2pm + secondsPerHour ||  // 15:00
            m_simulationTime == time_5pm + secondsPerHour) {  // 18:00
            m_rushHourLabel->clear();
        }
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

QString MainWindow::formatSimulationTime(int seconds) const {
    int hour = (seconds * 24) / m_dayDuration;
    int minute = ((seconds * 24 * 60) / m_dayDuration) % 60;
    QString timeStr = QString("%1:%2").arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0'));
    
    // 添加时段说明
    QString period;
    if (hour >= 5 && hour < 12) {
        period = "上午";
    } else if (hour >= 12 && hour < 13) {
        period = "中午";
    } else if (hour >= 13 && hour < 18) {
        period = "下午";
    } else if (hour >= 18 && hour < 22) {
        period = "晚上";
    } else {
        period = "深夜";
    }
    
    return QString("%1\n%2").arg(timeStr).arg(period);
} 