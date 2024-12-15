#include "energy_manager.h"
#include <sstream>
#include <iomanip>

EnergyManager::EnergyManager(size_t elevatorCount) {
    elevatorMetrics.resize(elevatorCount);
}

void EnergyManager::updateEnergy(const std::vector<Elevator>& elevators, double deltaTime) {
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        auto& metrics = elevatorMetrics[i];
        
        // 计算时间段内的能耗
        double hourFraction = deltaTime / 3600.0; // 转换为小时
        
        switch (elevator.getState()) {
            case ElevatorState::IDLE:
                metrics.idleConsumption += IDLE_POWER * hourFraction;
                break;
                
            case ElevatorState::MOVING_UP:
            case ElevatorState::MOVING_DOWN:
                metrics.movingConsumption += MOVING_POWER * hourFraction;
                break;
                
            case ElevatorState::STOPPED:
                metrics.doorOperations += 1;
                break;
        }
        
        metrics.totalConsumption = metrics.idleConsumption + 
                                 metrics.movingConsumption + 
                                 metrics.doorOperations * DOOR_OPERATION;
    }
}

std::string EnergyManager::getEnergyReport() const {
    std::stringstream ss;
    ss << "\n=== 能源消耗报告 ===" << std::endl;
    ss << std::fixed << std::setprecision(2);
    
    double totalConsumption = 0;
    for (size_t i = 0; i < elevatorMetrics.size(); ++i) {
        const auto& metrics = elevatorMetrics[i];
        totalConsumption += metrics.totalConsumption;
        
        ss << "电梯 " << (i + 1) << ":" << std::endl;
        ss << "  总能耗: " << metrics.totalConsumption << " kWh" << std::endl;
        ss << "  空闲能耗: " << metrics.idleConsumption << " kWh" << std::endl;
        ss << "  运行能耗: " << metrics.movingConsumption << " kWh" << std::endl;
        ss << "  开关门次数: " << static_cast<int>(metrics.doorOperations) << std::endl;
        ss << std::endl;
    }
    
    ss << "系统总能耗: " << totalConsumption << " kWh" << std::endl;
    return ss.str();
}

std::string EnergyManager::getEnergyOptimizationTips() const {
    std::stringstream ss;
    ss << "\n=== 节能优化建议 ===" << std::endl;
    
    // 分析空闲能耗比例
    double totalIdle = 0;
    double totalMoving = 0;
    for (const auto& metrics : elevatorMetrics) {
        totalIdle += metrics.idleConsumption;
        totalMoving += metrics.movingConsumption;
    }
    
    double totalConsumption = getTotalConsumption();
    if (totalConsumption > 0) {
        double idleRatio = totalIdle / totalConsumption;
        
        if (idleRatio > 0.3) {
            ss << "- 空闲能耗比例过高 (" << std::fixed << std::setprecision(1) 
               << (idleRatio * 100) << "%), 建议：" << std::endl;
            ss << "  * 考虑在非高峰时段减少运行电梯数量" << std::endl;
            ss << "  * 缩短电梯空闲等待时间" << std::endl;
        }
    }
    
    // 分析开关门频率
    for (size_t i = 0; i < elevatorMetrics.size(); ++i) {
        const auto& metrics = elevatorMetrics[i];
        if (metrics.doorOperations > 100) { // 假设阈值
            ss << "- 电梯 " << (i + 1) << " 开关门次数过多，建议：" << std::endl;
            ss << "  * 优化调度算法，减少不必要的停靠" << std::endl;
            ss << "  * 考虑实施分层停靠策略" << std::endl;
        }
    }
    
    return ss.str();
}

void EnergyManager::reset() {
    for (auto& metrics : elevatorMetrics) {
        metrics = EnergyMetrics();
    }
}

double EnergyManager::getTotalConsumption() const {
    double total = 0;
    for (const auto& metrics : elevatorMetrics) {
        total += metrics.totalConsumption;
    }
    return total;
} 