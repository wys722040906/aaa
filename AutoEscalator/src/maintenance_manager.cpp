#include "maintenance_manager.h"
#include <sstream>
#include <iomanip>
#include <random>

MaintenanceManager::MaintenanceManager(size_t elevatorCount) {
    elevatorStatus.resize(elevatorCount);
}

void MaintenanceManager::update(std::vector<Elevator>& elevators, double currentTime) {
    for (size_t i = 0; i < elevators.size(); ++i) {
        auto& status = elevatorStatus[i];
        auto& elevator = elevators[i];
        
        // 更新操作计数
        if (elevator.getState() != ElevatorState::IDLE) {
            status.operationCount++;
        }
        
        // 检查是否需要维护
        if (status.operationCount >= OPERATIONS_BEFORE_MAINTENANCE ||
            (currentTime - status.lastMaintenanceTime) >= MAINTENANCE_INTERVAL) {
            status.needsMaintenance = true;
            maintenanceQueue.push(i);
        }
        
        // 模拟随机故障
        if (!status.hasFault && !status.needsMaintenance) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<> dis(0, 1);
            
            if (dis(gen) < FAULT_PROBABILITY) {
                simulateFault(i);
            }
        }
        
        // 如果有故障或需要维护，停止电梯
        if (status.hasFault || status.needsMaintenance) {
            elevator.setState(ElevatorState::STOPPED);
        }
    }
}

void MaintenanceManager::simulateFault(int elevatorId) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 4);
    
    auto& status = elevatorStatus[elevatorId];
    status.hasFault = true;
    status.currentFault = static_cast<FaultType>(dis(gen));
    
    std::string faultDesc;
    switch (status.currentFault) {
        case FaultType::DOOR_MALFUNCTION:
            faultDesc = "门故障";
            break;
        case FaultType::SENSOR_ERROR:
            faultDesc = "传感器错误";
            break;
        case FaultType::OVERLOAD:
            faultDesc = "超载";
            break;
        case FaultType::POWER_FAILURE:
            faultDesc = "电源故障";
            break;
        case FaultType::COMMUNICATION_ERROR:
            faultDesc = "通信错误";
            break;
    }
    
    maintenanceHistory.emplace_back(elevatorId, "故障", 0.0, faultDesc);
}

void MaintenanceManager::performMaintenance(int elevatorId, double currentTime) {
    auto& status = elevatorStatus[elevatorId];
    status.lastMaintenanceTime = currentTime;
    status.operationCount = 0;
    status.needsMaintenance = false;
    status.hasFault = false;
    
    maintenanceHistory.emplace_back(elevatorId, "定期维护", currentTime, 
                                  "完成例行维护检查");
}

std::string MaintenanceManager::getMaintenanceReport() const {
    std::stringstream ss;
    ss << "\n=== 维护状态报告 ===" << std::endl;
    
    for (size_t i = 0; i < elevatorStatus.size(); ++i) {
        const auto& status = elevatorStatus[i];
        ss << "电梯 " << (i + 1) << ":" << std::endl;
        ss << "  运行次数: " << status.operationCount << std::endl;
        ss << "  上次维护时间: " << std::fixed << std::setprecision(1) 
           << status.lastMaintenanceTime << "s" << std::endl;
        
        if (status.hasFault) {
            ss << "  当前状态: 故障" << std::endl;
            ss << "  故障类型: " << getFaultTypeString(status.currentFault) << std::endl;
        } else if (status.needsMaintenance) {
            ss << "  当前状态: 需要维护" << std::endl;
        } else {
            ss << "  当前状态: 正常" << std::endl;
        }
        ss << std::endl;
    }
    
    return ss.str();
}

std::string MaintenanceManager::getFaultTypeString(FaultType type) const {
    switch (type) {
        case FaultType::DOOR_MALFUNCTION: return "门故障";
        case FaultType::SENSOR_ERROR: return "传感器错误";
        case FaultType::OVERLOAD: return "超载";
        case FaultType::POWER_FAILURE: return "电源故障";
        case FaultType::COMMUNICATION_ERROR: return "通信错误";
        default: return "未知故障";
    }
}

void MaintenanceManager::reset() {
    for (auto& status : elevatorStatus) {
        status = ElevatorStatus();
    }
    maintenanceHistory.clear();
    while (!maintenanceQueue.empty()) {
        maintenanceQueue.pop();
    }
}

bool MaintenanceManager::needsMaintenance(int elevatorId) const {
    if (elevatorId < 0 || elevatorId >= static_cast<int>(elevatorStatus.size())) {
        return false;
    }
    return elevatorStatus[elevatorId].needsMaintenance;
}

const std::vector<MaintenanceManager::MaintenanceRecord>& 
MaintenanceManager::getMaintenanceHistory() const {
    return maintenanceHistory;
}

void MaintenanceManager::repairFault(int elevatorId, double currentTime) {
    if (elevatorId < 0 || elevatorId >= static_cast<int>(elevatorStatus.size())) {
        return;
    }
    
    auto& status = elevatorStatus[elevatorId];
    if (status.hasFault) {
        status.hasFault = false;
        maintenanceHistory.emplace_back(elevatorId, "维修", currentTime,
            "修复" + getFaultTypeString(status.currentFault) + "故障");
    }
} 