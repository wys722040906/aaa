#pragma once
#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include "Elevator.h"
#include <QObject>

class ElevatorDispatcher : public QObject {
    Q_OBJECT
public:
    struct Request {
        int fromFloor;
        int toFloor;
        bool isEmergency;
        long long timestamp;
        
        bool operator<(const Request& other) const {
            if (isEmergency != other.isEmergency) {
                return !isEmergency; // 紧急请求优先级更高
            }
            return timestamp > other.timestamp;
        }
    };

    explicit ElevatorDispatcher(QObject* parent = nullptr);
    
    // 核心调度方法
    void dispatchRequest(int fromFloor, int toFloor, bool isEmergency = false);
    
    // 电梯管理
    void addElevator(Elevator* elevator);
    void setElevatorRange(int elevatorId, int minFloor, int maxFloor);
    
    // 模式控制
    void setEmergencyMode(bool enabled);
    void setPeakHourMode(bool enabled);
    
    // 统计信息
    void updateFloorStatistics(int floor);
    std::vector<int> getHotFloors() const;
    
    int getElevatorCount() const { return elevators.size(); }
    Elevator* getElevator(int index) const {
        return (index >= 0 && index < elevators.size()) ? elevators[index] : nullptr;
    }

    void setElevatorSpeed(int elevatorId, double speed);
    void initializeElevators();

private:
    // 调度算法辅助方法
    Elevator* findBestElevator(const Request& request);
    int calculateScore(Elevator* elevator, const Request& request);
    bool canServeRequest(Elevator* elevator, const Request& request);
    
    std::vector<Elevator*> elevators;
    std::priority_queue<Request> requestQueue;
    std::unordered_map<int, int> floorStatistics;
    
    bool emergencyMode{false};
    bool peakHourMode{false};
    void processQueue();
}; 