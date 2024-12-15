#pragma once
#include "elevator.h"
#include <vector>
#include <map>

class EnergyManager {
private:
    struct EnergyMetrics {
        double totalConsumption;    // 总能耗(kWh)
        double idleConsumption;     // 空闲能耗
        double movingConsumption;   // 运行能耗
        double doorOperations;      // 开关门次数
        
        EnergyMetrics() : totalConsumption(0), idleConsumption(0),
                         movingConsumption(0), doorOperations(0) {}
    };
    
    std::vector<EnergyMetrics> elevatorMetrics;
    
    // 能耗参数
    static constexpr double IDLE_POWER = 0.1;     // 空闲功率(kW)
    static constexpr double MOVING_POWER = 2.5;    // 运行功率(kW)
    static constexpr double DOOR_OPERATION = 0.05; // 每次开关门能耗(kWh)
    
public:
    explicit EnergyManager(size_t elevatorCount);
    
    // 更新能耗统计
    void updateEnergy(const std::vector<Elevator>& elevators, double deltaTime);
    
    // 获取能耗报告
    std::string getEnergyReport() const;
    
    // 获取节能建议
    std::string getEnergyOptimizationTips() const;
    
    // 重置统计数据
    void reset();
    
    // 获取总能耗
    double getTotalConsumption() const;
}; 