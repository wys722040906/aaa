#include "dispatcher.h"
#include <algorithm>
#include <limits>

Dispatcher::Dispatcher(Strategy strategy) 
    : currentStrategy(strategy), stats{0, 0, 0.0, 0.0} {}

void Dispatcher::setStrategy(Strategy strategy) {
    currentStrategy = strategy;
}

Dispatcher::Strategy Dispatcher::getStrategy() const {
    return currentStrategy;
}

void Dispatcher::resetStatistics() {
    stats = {0, 0, 0.0, 0.0};
}

const Dispatcher::Statistics& Dispatcher::getStatistics() const {
    return stats;
}

int Dispatcher::assignElevator(const std::vector<Elevator>& elevators, 
                             const Passenger& passenger) {
    stats.totalAssignments++;
    int assignedElevator = -1;
    
    switch (currentStrategy) {
        case Strategy::NEAREST_FIRST:
            assignedElevator = assignNearestElevator(elevators, passenger);
            break;
            
        case Strategy::LOAD_BALANCED:
            assignedElevator = assignLoadBalancedElevator(elevators, passenger);
            break;
            
        case Strategy::ENERGY_SAVING:
            assignedElevator = assignEnergySavingElevator(elevators, passenger);
            break;
    }
    
    if (assignedElevator >= 0) {
        stats.successfulAssignments++;
        stats.averageWaitTime = (stats.averageWaitTime * (stats.successfulAssignments - 1) + 
                                passenger.getWaitTime()) / stats.successfulAssignments;
        
        int distance = std::abs(elevators[assignedElevator].getCurrentFloor() - 
                              passenger.getSourceFloor());
        stats.averageDistance = (stats.averageDistance * (stats.successfulAssignments - 1) + 
                               distance) / stats.successfulAssignments;
    }
    
    return assignedElevator;
}

int Dispatcher::assignNearestElevator(const std::vector<Elevator>& elevators, 
                                    const Passenger& passenger) {
    int bestElevator = -1;
    int minDistance = std::numeric_limits<int>::max();
    
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        if (elevator.getCurrentLoad() >= elevator.getCapacity()) {
            continue;
        }
        
        int distance = std::abs(elevator.getCurrentFloor() - passenger.getSourceFloor());
        if (distance < minDistance) {
            minDistance = distance;
            bestElevator = i;
        }
    }
    
    return bestElevator;
}

int Dispatcher::assignLoadBalancedElevator(const std::vector<Elevator>& elevators, 
                                         const Passenger& passenger) {
    int bestElevator = -1;
    double minLoad = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        if (elevator.getCurrentLoad() >= elevator.getCapacity()) {
            continue;
        }
        
        // 计算负载因子（考虑当前载客量和距离）
        double loadFactor = (double)elevator.getCurrentLoad() / elevator.getCapacity() +
                          std::abs(elevator.getCurrentFloor() - passenger.getSourceFloor()) * 0.1;
                          
        if (loadFactor < minLoad) {
            minLoad = loadFactor;
            bestElevator = i;
        }
    }
    
    return bestElevator;
}

int Dispatcher::assignEnergySavingElevator(const std::vector<Elevator>& elevators, 
                                         const Passenger& passenger) {
    int bestElevator = -1;
    int minEnergyCost = std::numeric_limits<int>::max();
    
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        if (elevator.getCurrentLoad() >= elevator.getCapacity()) {
            continue;
        }
        
        // 计算能耗（考虑移动距离和当前状态）
        int energyCost = std::abs(elevator.getCurrentFloor() - passenger.getSourceFloor());
        if (elevator.getState() == ElevatorState::IDLE) {
            energyCost *= 2; // 从空闲状态启动需要更多能耗
        }
        
        if (energyCost < minEnergyCost) {
            minEnergyCost = energyCost;
            bestElevator = i;
        }
    }
    
    return bestElevator;
}

std::string Dispatcher::getStrategyName(Strategy strategy) {
    switch (strategy) {
        case Strategy::NEAREST_FIRST: return "最近优先";
        case Strategy::LOAD_BALANCED: return "负载均衡";
        case Strategy::ENERGY_SAVING: return "节能模式";
        default: return "未知策略";
    }
} 