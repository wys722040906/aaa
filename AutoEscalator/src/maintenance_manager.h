#pragma once
#include "elevator.h"
#include <vector>
#include <string>
#include <queue>
#include <chrono>

class MaintenanceManager {
public:
    struct MaintenanceRecord {
        int elevatorId;
        std::string type;
        double timestamp;
        std::string description;
        
        MaintenanceRecord(int id, const std::string& t, double time, const std::string& desc)
            : elevatorId(id), type(t), timestamp(time), description(desc) {}
    };
    
    enum class FaultType {
        DOOR_MALFUNCTION,
        SENSOR_ERROR,
        OVERLOAD,
        POWER_FAILURE,
        COMMUNICATION_ERROR
    };
    
private:
    struct ElevatorStatus {
        double lastMaintenanceTime;
        int operationCount;
        bool needsMaintenance;
        bool hasFault;
        FaultType currentFault;
        
        ElevatorStatus() : lastMaintenanceTime(0), operationCount(0),
                          needsMaintenance(false), hasFault(false) {}
    };
    
    std::vector<ElevatorStatus> elevatorStatus;
    std::vector<MaintenanceRecord> maintenanceHistory;
    std::queue<int> maintenanceQueue;
    
    // 维护参数
    static constexpr int OPERATIONS_BEFORE_MAINTENANCE = 1000;  // 需要维护的操作次数
    static constexpr double MAINTENANCE_INTERVAL = 24 * 3600;   // 维护间隔（秒）
    static constexpr double FAULT_PROBABILITY = 0.001;         // 故障概率
    
public:
    explicit MaintenanceManager(size_t elevatorCount);
    
    // 更新电梯状态
    void update(std::vector<Elevator>& elevators, double currentTime);
    
    // 检查是否需要维护
    bool needsMaintenance(int elevatorId) const;
    
    // 执行维护
    void performMaintenance(int elevatorId, double currentTime);
    
    // 获取维护历史
    const std::vector<MaintenanceRecord>& getMaintenanceHistory() const;
    
    // 获取维护状态报告
    std::string getMaintenanceReport() const;
    
    // 模拟故障
    void simulateFault(int elevatorId);
    
    // 修复故障
    void repairFault(int elevatorId, double currentTime);
    
    // 重置
    void reset();
}; 