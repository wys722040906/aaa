#include "core/ElevatorDispatcher.h"
#include <algorithm>
#include <chrono>

ElevatorDispatcher::ElevatorDispatcher(QObject* parent)
    : QObject(parent) {
    elevators.reserve(4);
}

void ElevatorDispatcher::dispatchRequest(int fromFloor, int toFloor, bool isEmergency) {
    Request request{
        fromFloor,
        toFloor,
        isEmergency,
        std::chrono::system_clock::now().time_since_epoch().count()
    };
    
    // 如果是紧急请求，直接处理
    if (isEmergency) {
        if (Elevator* elevator = findBestElevator(request)) {
            elevator->addDestination(fromFloor);
            elevator->addDestination(toFloor);
        }
        return;
    }
    
    // 普通请求加入队列
    requestQueue.push(request);
    processQueue();
}

Elevator* ElevatorDispatcher::findBestElevator(const Request& request) {
    Elevator* bestElevator = nullptr;
    int bestScore = std::numeric_limits<int>::max();
    
    for (Elevator* elevator : elevators) {
        if (!canServeRequest(elevator, request)) {
            continue;
        }
        
        int score = calculateScore(elevator, request);
        if (score < bestScore) {
            bestScore = score;
            bestElevator = elevator;
        }
    }
    
    return bestElevator;
}

int ElevatorDispatcher::calculateScore(Elevator* elevator, const Request& request) {
    int score = 0;
    
    // 基础分：电梯当前位置到请求起点的距离
    score += std::abs(elevator->getCurrentFloor() - request.fromFloor);
    
    // 电梯状态加权
    switch (elevator->getState()) {
        case Elevator::State::IDLE:
            score += 0;
            break;
        case Elevator::State::MOVING_UP:
            score += (request.fromFloor < elevator->getCurrentFloor()) ? 50 : 0;
            break;
        case Elevator::State::MOVING_DOWN:
            score += (request.fromFloor > elevator->getCurrentFloor()) ? 50 : 0;
            break;
        default:
            score += 100;
    }
    
    // 高峰期优化
    if (peakHourMode) {
        auto hotFloors = getHotFloors();
        if (std::find(hotFloors.begin(), hotFloors.end(), request.fromFloor) != hotFloors.end()) {
            score -= 20;  // 热点楼层优先级提高
        }
    }
    
    // 乘客数量影响
    score += elevator->getPassengerCount() * 10;
    
    return score;
}

bool ElevatorDispatcher::canServeRequest(Elevator* elevator, const Request& request) {
    // 检查电梯是否在工作区间内
    const auto& config = elevator->getConfig();
    if (request.fromFloor < config.minFloor || request.fromFloor > config.maxFloor ||
        request.toFloor < config.minFloor || request.toFloor > config.maxFloor) {
        return false;
    }
    
    // 检查电梯状态
    if (elevator->getState() == Elevator::State::MALFUNCTION ||
        elevator->getState() == Elevator::State::EMERGENCY) {
        return false;
    }
    
    // 检查是否超载
    if (elevator->isOverloaded()) {
        return false;
    }
    
    return true;
}

void ElevatorDispatcher::updateFloorStatistics(int floor) {
    floorStatistics[floor]++;
}

std::vector<int> ElevatorDispatcher::getHotFloors() const {
    std::vector<std::pair<int, int>> floors;
    for (const auto& pair : floorStatistics) {
        floors.emplace_back(pair);
    }
    
    // 按访问频率排序
    std::sort(floors.begin(), floors.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // 返回前3个热点楼层
    std::vector<int> hotFloors;
    for (size_t i = 0; i < std::min(size_t(3), floors.size()); ++i) {
        hotFloors.push_back(floors[i].first);
    }
    
    return hotFloors;
}

void ElevatorDispatcher::processQueue() {
    while (!requestQueue.empty()) {
        Request request = requestQueue.top();
        
        if (Elevator* elevator = findBestElevator(request)) {
            elevator->addDestination(request.fromFloor);
            elevator->addDestination(request.toFloor);
            requestQueue.pop();
        } else {
            // 如果没有合适的电梯，保留请求在队列中
            break;
        }
    }
}

void ElevatorDispatcher::setElevatorRange(int elevatorId, int minFloor, int maxFloor) {
    if (elevatorId >= 0 && elevatorId < elevators.size()) {
        if (auto elevator = elevators[elevatorId]) {
            Elevator::Config config = elevator->getConfig();
            config.minFloor = minFloor;
            config.maxFloor = maxFloor;
            elevator->setConfig(config);
        }
    }
}

void ElevatorDispatcher::setEmergencyMode(bool enabled) {
    emergencyMode = enabled;
    for (auto elevator : elevators) {
        if (elevator) {
            if (enabled) {
                elevator->handleEmergency();
            }
        }
    }
}

void ElevatorDispatcher::setPeakHourMode(bool enabled) {
    peakHourMode = enabled;
}

void ElevatorDispatcher::addElevator(Elevator* elevator) {
    if (elevator) {
        elevators.push_back(elevator);
    }
}