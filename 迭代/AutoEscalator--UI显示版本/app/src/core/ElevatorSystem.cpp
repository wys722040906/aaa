#include "ElevatorSystem.h"

void ElevatorSystem::update() {
    // 更新所有电梯状态
    for (auto& elevator : elevators) {
        elevator.updateState();
    }
}

void ElevatorSystem::handleRequests() {
    // TODO: 实现请求处理逻辑
} 