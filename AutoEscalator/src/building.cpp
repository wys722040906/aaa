#include "building.h"
#include <algorithm>

Building::Building() 
    : dispatcher(Dispatcher::Strategy::NEAREST_FIRST),
      energyManager(ELEVATOR_COUNT),
      maintenanceManager(ELEVATOR_COUNT) {
    // 初始化电梯
    elevators.resize(ELEVATOR_COUNT, Elevator());
    // 初始化每层楼的等待队列
    waitingPassengers.resize(FLOOR_COUNT + 1); // +1因为从1楼开始计数
    
    // 开始记录数据
    dataRecorder.startLogging("elevator_data.csv");
}

void Building::update(double deltaTime) {
    performance.startMeasure("elevator_updates");
    // 更新所有电梯
    for (auto& elevator : elevators) {
        elevator.update(deltaTime);
    }
    performance.endMeasure("elevator_updates");
    
    performance.startMeasure("passenger_updates");
    // 更新等待乘客并处理超时
    for (int floor = 1; floor <= FLOOR_COUNT; ++floor) {
        auto& queue = waitingPassengers[floor];
        std::queue<Passenger> newQueue;
        
        while (!queue.empty()) {
            Passenger passenger = queue.front();
            queue.pop();
            
            passenger.updateWaitTime(deltaTime);
            if (!passenger.hasTimeout()) {
                newQueue.push(passenger);
            } else {
                Logger::log("乘客在" + std::to_string(floor) + "楼等待超时，已离开");
            }
        }
        
        waitingPassengers[floor] = newQueue;
    }
    performance.endMeasure("passenger_updates");
    
    performance.startMeasure("passenger_assignment");
    // 分配乘客到电梯
    assignPassengersToElevators();
    performance.endMeasure("passenger_assignment");
    
    // 更新能源统计
    energyManager.updateEnergy(elevators, deltaTime);
    
    // 更新维护状态
    maintenanceManager.update(elevators, currentTime);
    
    // 记录电梯状态
    dataRecorder.recordState(elevators, currentTime);
}

void Building::addRequest(int fromFloor, int toFloor, int passengerCount) {
    if (fromFloor < 1 || fromFloor > FLOOR_COUNT || 
        toFloor < 1 || toFloor > FLOOR_COUNT || 
        fromFloor == toFloor) {
        return;
    }
    
    for (int i = 0; i < passengerCount; ++i) {
        waitingPassengers[fromFloor].push(Passenger(fromFloor, toFloor));
    }
}

void Building::reset() {
    // 重置所有电梯
    for (auto& elevator : elevators) {
        elevator.reset();
    }
    
    // 清空等待队列
    for (auto& queue : waitingPassengers) {
        while (!queue.empty()) {
            queue.pop();
        }
    }
    
    // 清除记录数据
    dataRecorder.clear();
}

const std::vector<Elevator>& Building::getElevators() const {
    return elevators;
}

const std::vector<std::queue<Passenger>>& Building::getWaitingPassengers() const {
    return waitingPassengers;
}

void Building::assignPassengersToElevators() {
    for (int floor = 1; floor <= FLOOR_COUNT; ++floor) {
        auto& queue = waitingPassengers[floor];
        if (queue.empty()) continue;
        
        while (!queue.empty()) {
            const auto& passenger = queue.front();
            int elevatorIndex = dispatcher.assignElevator(elevators, passenger);
            
            if (elevatorIndex >= 0) {
                auto& elevator = elevators[elevatorIndex];
                if (elevator.addPassenger(passenger)) {
                    // 设置电梯运动方向
                    if (elevator.getCurrentFloor() < floor) {
                        elevator.setState(ElevatorState::MOVING_UP);
                    } else if (elevator.getCurrentFloor() > floor) {
                        elevator.setState(ElevatorState::MOVING_DOWN);
                    }
                    queue.pop();
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }
}

void Building::displayWaitingPassengers() const {
    std::cout << "\n=== 等待乘客状态 ===" << std::endl;
    for (int floor = 1; floor <= FLOOR_COUNT; ++floor) {
        if (!waitingPassengers[floor].empty()) {
            std::cout << floor << "楼: " << waitingPassengers[floor].size() 
                     << "人等待" << std::endl;
        }
    }
}

int Building::getTotalWaitingPassengers() const {
    int total = 0;
    for (const auto& queue : waitingPassengers) {
        total += queue.size();
    }
    return total;
}

int Building::getWaitingCountAtFloor(int floor) const {
    if (floor < 1 || floor > FLOOR_COUNT) {
        return 0;
    }
    return waitingPassengers[floor].size();
}

void Building::setDispatchStrategy(Dispatcher::Strategy strategy) {
    dispatcher.setStrategy(strategy);
    dispatcher.resetStatistics();
}

Dispatcher::Strategy Building::getDispatchStrategy() const {
    return dispatcher.getStrategy();
}

const Dispatcher::Statistics& Building::getDispatcherStatistics() const {
    return dispatcher.getStatistics();
}

const EnergyManager& Building::getEnergyManager() const {
    return energyManager;
}

const MaintenanceManager& Building::getMaintenanceManager() const {
    return maintenanceManager;
}

const DataRecorder& Building::getDataRecorder() const {
    return dataRecorder;
} 