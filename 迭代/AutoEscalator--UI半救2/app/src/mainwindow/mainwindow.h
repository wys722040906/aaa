#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QMenu>
#include <QMenuBar>
#include <QVector>
#include <QTimer>
#include "../floor/floorwidget.h"  // 添加FloorWidget头文件
#include "../elevator/elevatorwidget.h"  // 改为包含而不是前向声明
#include "../controller/elevatorcontroller.h"  // 添加这行
#include "../settings/settingsdialog.h"  // 改为包含而不是前向声明
#include "../statistics/statisticsdialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onResetClicked();
    void onSettingsTriggered();
    void onLogoutTriggered();
    void updateSimulation();  // 更新模拟状态
    void checkRushHour();    // 检查高峰时段

private:
    void setupUI();
    void setupMenuBar();
    void setupControlPanel();
    void setupFloorDisplay();
    void setupElevatorDisplay();
    void setupConnections();
    void createRushHourPassengers(bool isGroundFloor, const QString& type);

    // UI组件
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    QVBoxLayout* m_controlLayout;
    QWidget* m_floorDisplayArea;
    QWidget* m_elevatorDisplayArea;
    
    QPushButton* m_startButton;
    QPushButton* m_resetButton;
    QLabel* m_rushHourLabel;
    
    QVector<FloorWidget*> m_floorWidgets;      // 14个楼层显示组件
    QVector<ElevatorWidget*> m_elevatorWidgets; // 4个电梯显示组件
    
    SettingsDialog* m_settingsDialog;
    StatisticsDialog* m_statisticsDialog;
    
    // 系统状态
    bool m_isRunning;
    int m_simulationTime;    // 模拟时间（秒）
    QTimer* m_simulationTimer;
    QTimer* m_passengerGenerator;
    
    // 配置参数
    int m_dayDuration;       // 一天的持续时间（秒）
    int m_floorTravelTime;   // 电梯经过一层的时间（秒）
    int m_maxPassengers;     // 电梯最大载客数
    int m_idleTime;         // 电梯空闲等待时间
    int m_requestInterval;   // 随机乘客请求生成间隔
    
    // 添加电梯控制器
    ElevatorController* m_elevatorController;
};

#endif // MAINWINDOW_H 