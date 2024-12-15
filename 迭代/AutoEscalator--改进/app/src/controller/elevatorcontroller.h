#ifndef ELEVATORCONTROLLER_H
#define ELEVATORCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QMap>
#include <QDebug>
#include "../elevator/elevatorwidget.h"
#include "../common/types.h"

// 电梯状态结构体
struct ElevatorStatus {
    int currentFloor;           // 当前楼层
    Direction direction;        // 运行方向
    int passengerCount;        // 当前载客数
    bool isMoving;             // 是否在移动
    qint64 lastStopTime;       // 上次停靠时间
    QVector<int> targetFloors; // 目标楼层列表
};

class ElevatorController : public QObject {
    Q_OBJECT

public:
    explicit ElevatorController(QVector<ElevatorWidget*>& elevators, QObject *parent = nullptr);
    ~ElevatorController() = default;

    // 添加乘客请求
    void addRequest(int currentFloor, int targetFloor, int passengerCount);
    // 添加高峰时段请求
    void addRushHourRequests(bool isGroundFloor, const QString& type);
    // 重置所有电梯状态
    void reset();
    // 设置系统参数
    void setParameters(int maxPassengers, int floorTravelTime, int idleTime);
    void generateRandomPassengers();  // 添加公共方法声明

    // 设置乘客生成数量
    void setRandomPassengerCount(int count) { 
        m_randomPassengerCount = count; 
        qDebug() << "Random passenger count updated:" << count;
    }
    
    void setPeakPassengerCount(int count) { 
        m_peakPassengerCount = count; 
        qDebug() << "Peak passenger count updated:" << count;
    }

    // 添加设置开关门时间的方法
    void setDoorTime(int milliseconds);

public slots:
    void update(); // 更新电梯状态

signals:
    void statisticsUpdated(int floor, int totalPassengers);
    void floorRequestCompleted(int fromFloor, int toFloor, int count);
    void floorRequestAdded(int fromFloor, int toFloor, int count);  // 添加新信号

private:
    void updateElevatorStatus(int elevatorId);
    bool assignRequestToElevator(const PassengerRequest& request);  // 改为返回 bool
    int findBestElevator(const PassengerRequest& request) const;
    bool canElevatorTakeRequest(int elevatorId, const PassengerRequest& request) const;
    void moveElevator(int elevatorId);
    void handleElevatorArrival(int elevatorId);
    bool shouldStopAtFloor(int elevatorId, int floor) const;
    Direction determineDirection(int elevatorId) const;
    void updateStatistics(int floor, int passengerCount);
    void processRequests();
    void insertSortedFloor(int elevatorId, int floor);  // 添加新的私有方法

    // 添加处理乘客上下电梯的函数声明
    void processPassengersAtFloor(int elevatorId);

    // 成员变量
    QVector<ElevatorWidget*>& m_elevators;
    QVector<ElevatorStatus> m_elevatorStatus;
    QQueue<PassengerRequest> m_pendingRequests;
    QMap<int, QVector<PassengerRequest>> m_assignedRequests; // elevatorId -> requests
    QMap<int, int> m_floorStatistics; // floor -> usage count

    // 系统参数
    int m_maxPassengers;
    int m_floorTravelTime;
    int m_idleTime;
    int m_randomPassengerCount;  // 普通时段每次生成的乘客数量
    int m_peakPassengerCount;    // 高峰期每次生成的乘客数量
    bool m_isFirstRequest;  // 添加标记变量
    QTimer* m_requestTimer;  // 添加定时器
    int m_doorTime;  // 开关门时间（毫秒）
    QTimer* m_doorTimer;  // 开关门计时器
    int m_currentElevatorId;  // 当前正在处理的电梯ID
};

#endif // ELEVATORCONTROLLER_H 