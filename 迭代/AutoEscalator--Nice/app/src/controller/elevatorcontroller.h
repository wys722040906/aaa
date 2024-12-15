#ifndef ELEVATORCONTROLLER_H
#define ELEVATORCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QMap>
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
};

#endif // ELEVATORCONTROLLER_H 