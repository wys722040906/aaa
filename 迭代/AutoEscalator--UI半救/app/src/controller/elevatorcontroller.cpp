#include "elevatorcontroller.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

ElevatorController::ElevatorController(QVector<ElevatorWidget*>& elevators, QObject *parent)
    : QObject(parent)
    , m_elevators(elevators)
    , m_maxPassengers(12)
    , m_floorTravelTime(5)
    , m_idleTime(10)
{
    // 检查电梯数量
    if (m_elevators.isEmpty()) {
        qDebug() << "Warning: No elevators provided to ElevatorController";
        return;
    }

    // 初始化电梯状态
    m_elevatorStatus.resize(m_elevators.size());
    for (int i = 0; i < m_elevators.size(); ++i) {
        m_elevatorStatus[i] = {1, Direction::IDLE, 0, false, 0, QVector<int>()};
    }
}

void ElevatorController::setParameters(int maxPassengers, int floorTravelTime, int idleTime) {
    m_maxPassengers = maxPassengers;
    m_floorTravelTime = floorTravelTime;
    m_idleTime = idleTime;
    
    qDebug() << "Parameters updated - Max passengers:" << maxPassengers
             << "Floor travel time:" << floorTravelTime
             << "Idle time:" << idleTime;
}

void ElevatorController::addRequest(int currentFloor, int targetFloor, int passengerCount) {
    PassengerRequest request = {
        currentFloor,
        targetFloor,
        passengerCount,
        QDateTime::currentMSecsSinceEpoch()
    };
    
    // 尝试立即分配请求给最合适的电梯
    assignRequestToElevator(request);
}

void ElevatorController::addRushHourRequests(bool isGroundFloor, const QString& type) {
    // 生成10-15个随机请求
    int requestCount = QRandomGenerator::global()->bounded(10, 16);
    
    for (int i = 0; i < requestCount; ++i) {
        int currentFloor, targetFloor;
        if (isGroundFloor) {
            currentFloor = 1;
            targetFloor = QRandomGenerator::global()->bounded(2, 15); // 2-14楼
        } else {
            currentFloor = QRandomGenerator::global()->bounded(2, 15); // 2-14楼
            targetFloor = 1;
        }
        
        int passengerCount = QRandomGenerator::global()->bounded(1, 6); // 1-5人
        addRequest(currentFloor, targetFloor, passengerCount);
    }
}

void ElevatorController::update() {
    // 更新每部电梯的状态
    for (int i = 0; i < m_elevators.size(); ++i) {
        auto& status = m_elevatorStatus[i];
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;  // 转换为秒
        
        // 检查是否超过空闲时间
        if (status.direction == Direction::IDLE && 
            !status.isMoving && 
            (currentTime - status.lastStopTime) >= m_idleTime) {
            // 如果超过空闲时间，移动到1楼
            if (status.currentFloor != 1) {
                status.targetFloors.clear();
                status.targetFloors.append(1);
                status.direction = Direction::DOWN;
            }
        }
        
        updateElevatorStatus(i);
    }
    
    // 处理未分配的请求
    while (!m_pendingRequests.isEmpty()) {
        PassengerRequest request = m_pendingRequests.head();
        int bestElevator = findBestElevator(request);
        
        if (bestElevator >= 0) {
            m_pendingRequests.dequeue();
            m_assignedRequests[bestElevator].append(request);
        } else {
            break; // 没有合适的电梯，等待下次更新
        }
    }
}

void ElevatorController::updateElevatorStatus(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    auto elevator = m_elevators[elevatorId];
    
    if (status.isMoving) {
        // 检查是否需要在当前楼层停靠
        if (shouldStopAtFloor(elevatorId, status.currentFloor)) {
            handleElevatorArrival(elevatorId);
        } else {
            moveElevator(elevatorId);
        }
    } else {
        // 检查是否有新的目标楼层
        if (!status.targetFloors.isEmpty()) {
            status.direction = determineDirection(elevatorId);
            moveElevator(elevatorId);
        } else {
            // 空闲超时，返回一楼
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            if (status.currentFloor != 1 && 
                (currentTime - status.lastStopTime) / 1000 >= m_idleTime) {
                status.targetFloors.append(1);
                status.direction = status.currentFloor > 1 ? Direction::DOWN : Direction::UP;
                moveElevator(elevatorId);
            }
        }
    }
    
    // 更新电梯显示
    elevator->setCurrentFloor(status.currentFloor);
    elevator->setDirection(status.direction);
    elevator->setPassengerCount(status.passengerCount);
}

int ElevatorController::findBestElevator(const PassengerRequest& request) const {
    int bestElevator = -1;
    int minCost = INT_MAX;
    
    for (int i = 0; i < m_elevators.size(); ++i) {
        if (!canElevatorTakeRequest(i, request)) {
            continue;
        }
        
        // 计算成本（距离 + 当前负载）
        int distance = abs(m_elevatorStatus[i].currentFloor - request.currentFloor);
        int loadCost = m_elevatorStatus[i].passengerCount * 2; // 载客数越多，成本越高
        int totalCost = distance + loadCost;
        
        if (totalCost < minCost) {
            minCost = totalCost;
            bestElevator = i;
        }
    }
    
    return bestElevator;
}

bool ElevatorController::canElevatorTakeRequest(int elevatorId, const PassengerRequest& request) const {
    const auto& status = m_elevatorStatus[elevatorId];
    
    // 检查载客量
    if (status.passengerCount + request.passengerCount > m_maxPassengers) {
        return false;
    }
    
    // 检查方向兼容性
    if (status.direction != Direction::IDLE) {
        bool isRequestUpward = request.targetFloor > request.currentFloor;
        bool isElevatorUpward = status.direction == Direction::UP;
        
        if (isRequestUpward != isElevatorUpward) {
            return false;
        }
    }
    
    return true;
}

void ElevatorController::moveElevator(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    
    // 根据方向更新楼层
    if (status.direction == Direction::UP) {
        status.currentFloor++;
    } else if (status.direction == Direction::DOWN) {
        status.currentFloor--;
    }
    
    status.isMoving = true;
}

void ElevatorController::handleElevatorArrival(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    auto& requests = m_assignedRequests[elevatorId];
    
    // 处理到达目标楼层的乘客
    for (auto it = requests.begin(); it != requests.end();) {
        if (it->targetFloor == status.currentFloor) {
            status.passengerCount -= it->passengerCount;
            updateStatistics(status.currentFloor, it->passengerCount);
            
            // 通知对应楼层更新状态
            emit floorRequestCompleted(status.currentFloor, it->targetFloor, it->passengerCount);
            
            it = requests.erase(it);
        } else {
            ++it;
        }
    }
    
    // 更新电梯状态
    status.isMoving = false;
    status.lastStopTime = QDateTime::currentMSecsSinceEpoch();
    status.targetFloors.removeOne(status.currentFloor);
    
    if (status.targetFloors.isEmpty()) {
        status.direction = Direction::IDLE;
    }
    
    // 更新电梯显示
    m_elevators[elevatorId]->setPassengerCount(status.passengerCount);
}

Direction ElevatorController::determineDirection(int elevatorId) const {
    const auto& status = m_elevatorStatus[elevatorId];
    if (status.targetFloors.isEmpty()) {
        return Direction::IDLE;
    }
    
    int nextTarget = status.targetFloors.first();
    return nextTarget > status.currentFloor ? Direction::UP : Direction::DOWN;
}

bool ElevatorController::shouldStopAtFloor(int elevatorId, int floor) const {
    const auto& status = m_elevatorStatus[elevatorId];
    return status.targetFloors.contains(floor);
}

void ElevatorController::updateStatistics(int floor, int passengerCount) {
    m_floorStatistics[floor] += passengerCount;
    emit statisticsUpdated(floor, m_floorStatistics[floor]);
}

void ElevatorController::reset() {
    // 重置所有电梯状态
    for (int i = 0; i < m_elevators.size(); ++i) {
        m_elevatorStatus[i] = {1, Direction::IDLE, 0, false, 0, QVector<int>()};
        m_elevators[i]->setCurrentFloor(1);
        m_elevators[i]->setDirection(Direction::IDLE);
        m_elevators[i]->setPassengerCount(0);
    }
    
    // 清空请求队列和统计数据
    m_pendingRequests.clear();
    m_assignedRequests.clear();
    m_floorStatistics.clear();
}

void ElevatorController::assignRequestToElevator(const PassengerRequest& request) {
    // 找到最合适的电梯
    int bestElevator = findBestElevator(request);
    
    if (bestElevator >= 0) {
        // 如果找到合适的电梯，直接分配请求
        m_assignedRequests[bestElevator].append(request);
        
        // 更新电梯的目标楼层
        auto& status = m_elevatorStatus[bestElevator];
        if (!status.targetFloors.contains(request.currentFloor)) {
            status.targetFloors.append(request.currentFloor);
        }
        if (!status.targetFloors.contains(request.targetFloor)) {
            status.targetFloors.append(request.targetFloor);
        }
        
        // 如果电梯当前是空闲的，设置新的方向
        if (status.direction == Direction::IDLE) {
            status.direction = determineDirection(bestElevator);
        }
        
        // 更新电梯的载客数
        status.passengerCount += request.passengerCount;
        m_elevators[bestElevator]->setPassengerCount(status.passengerCount);
    } else {
        // 如果没有找到合适的电梯，将请求加入等待队列
        m_pendingRequests.enqueue(request);
    }
} 