#pragma once
#include <QObject>
#include <QTimer>
#include <unordered_map>
#include <queue>
#include <vector>
#include <memory>
#include "Passenger.h"
#include "ElevatorDispatcher.h"

class PassengerManager : public QObject {
    Q_OBJECT
public:
    explicit PassengerManager(ElevatorDispatcher* dispatcher, QObject* parent = nullptr);

    // 乘客管理
    void addPassenger(int fromFloor, int toFloor, Passenger::Type type = Passenger::Type::NORMAL);
    void removePassenger(int floor);
    
    // 高峰期模拟
    void startPeakHourSimulation();
    void stopPeakHourSimulation();
    
    // 楼层统计
    int getWaitingCount(int floor) const;
    std::vector<int> getBusiestFloors() const;
    
    // 乘客分布设置
    void setFloorPopulation(int floor, int workerCount, int elderlyCount);

private:
    struct FloorInfo {
        int workerCount{0};
        int elderlyCount{0};
        std::queue<std::shared_ptr<Passenger>> waitingQueue;
    };

    void simulatePeakHour();
    void generateRandomPassengers();
    int getRandomFloor() const;
    
    ElevatorDispatcher* dispatcher;
    std::unordered_map<int, FloorInfo> floorInfo;
    bool isPeakHour{false};
    
    // 定时器用于模拟高峰期
    QTimer* peakHourTimer;
}; 