#include "elevator.h"

Elevator::Elevator(int cap) 
    : currentFloor(1), capacity(cap), state(ElevatorState::IDLE), idleTime(0) {}

bool Elevator::addPassenger(const Passenger& passenger) {
    if (passengers.size() >= capacity) {
        return false;
    }
    passengers.push_back(passenger);
    return true;
}

void Elevator::removePassenger(int targetFloor) {
    passengers.erase(
        std::remove_if(passengers.begin(), passengers.end(),
            [targetFloor](const Passenger& p) {
                return p.getTargetFloor() == targetFloor;
            }
        ),
        passengers.end()
    );
}

void Elevator::move() {
    int oldFloor = currentFloor;
    
    switch (state) {
        case ElevatorState::MOVING_UP:
            currentFloor++;
            Logger::log("电梯从" + std::to_string(oldFloor) + "楼上行到" + 
                       std::to_string(currentFloor) + "楼");
            break;
            
        case ElevatorState::MOVING_DOWN:
            currentFloor--;
            Logger::log("电梯从" + std::to_string(oldFloor) + "楼下行到" + 
                       std::to_string(currentFloor) + "楼");
            break;
            
        default:
            break;
    }
}

void Elevator::update(double deltaTime) {
    // 更新电梯状态
    switch (state) {
        case ElevatorState::MOVING_UP:
        case ElevatorState::MOVING_DOWN:
            floorTravelTime += deltaTime;
            if (floorTravelTime >= FLOOR_TRAVEL_TIME) {
                move();
                floorTravelTime = 0.0;
                
                // 检查是否有乘客到达目标楼层
                bool hasPassengersToRemove = false;
                for (const auto& passenger : passengers) {
                    if (passenger.getTargetFloor() == currentFloor) {
                        hasPassengersToRemove = true;
                        break;
                    }
                }
                
                if (hasPassengersToRemove) {
                    state = ElevatorState::STOPPED;
                    removePassenger(currentFloor);
                }
            }
            break;
            
        case ElevatorState::IDLE:
            idleTime += deltaTime;
            if (idleTime >= MAX_IDLE_TIME && currentFloor != 1) {
                state = ElevatorState::MOVING_DOWN;
                idleTime = 0.0;
            }
            break;
            
        case ElevatorState::STOPPED:
            idleTime += deltaTime;
            if (idleTime >= 2.0) {  // 停靠2秒后继续运行
                if (!passengers.empty()) {
                    // 继续朝原来的方向运行
                    state = (passengers.front().getTargetFloor() > currentFloor) 
                        ? ElevatorState::MOVING_UP 
                        : ElevatorState::MOVING_DOWN;
                } else {
                    state = ElevatorState::IDLE;
                }
                idleTime = 0.0;
            }
            break;
    }
}

int Elevator::getCurrentFloor() const {
    return currentFloor;
}

int Elevator::getCurrentLoad() const {
    return passengers.size();
}

ElevatorState Elevator::getState() const {
    return state;
}

void Elevator::reset() {
    currentFloor = 1;
    passengers.clear();
    state = ElevatorState::IDLE;
    idleTime = 0;
}

void Elevator::setState(ElevatorState newState) {
    state = newState;
    if (state != ElevatorState::IDLE) {
        idleTime = 0;
    }
} 