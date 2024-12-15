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
    
    if (!assignRequestToElevator(request)) {
        // 如果分配失败，加入等待队列
        m_pendingRequests.enqueue(request);
    }
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
        updateElevatorStatus(i);
    }
    
    // 处理未分配的请求
    processRequests();
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
            // 如果有未处理的请求，继续移动
            moveElevator(elevatorId);
        } else {
            // 空闲超时，返回一楼
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
            if (status.currentFloor != 1 && 
                status.direction == Direction::IDLE &&
                (currentTime - status.lastStopTime) >= m_idleTime) {
                status.targetFloors.append(1);
                status.direction = status.currentFloor > 1 ? Direction::DOWN : Direction::UP;
                moveElevator(elevatorId);
            } else if (status.currentFloor == 1) {
                // 已经在一楼，保持空闲状态
                status.direction = Direction::IDLE;
                status.isMoving = false;
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
        
        const auto& status = m_elevatorStatus[i];
        
        // 基础成本：距离
        int distance = abs(status.currentFloor - request.currentFloor);
        
        // 负载成本：当前乘客数量
        int loadCost = status.passengerCount * 2;
        
        // 方向兼容性成本
        int directionCost = 0;
        bool isRequestUpward = request.targetFloor > request.currentFloor;
        
        if (status.direction != Direction::IDLE) {
            bool isElevatorUpward = status.direction == Direction::UP;
            // 如果方向相同，降低成本
            if (isRequestUpward == isElevatorUpward) {
                directionCost = 0;
            } else {
                directionCost = 10;  // 方向不同时增加成本
            }
        }
        
        // 计算总成本
        int totalCost = distance + loadCost + directionCost;
        
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
        
        // 如果电梯正在移动，检查是否能顺路接客
        if (isElevatorUpward) {
            // 电梯向上时：
            // 1. 请求楼层在当前楼层之上
            // 2. 如果请求也是向上的，则接受请求
            if (request.currentFloor < status.currentFloor ||
                (request.currentFloor > status.currentFloor && !isRequestUpward)) {
                return false;
            }
        } else {
            // 电梯向下时：
            // 1. 请求楼层在当前楼层之下
            // 2. 如果请求也是向下的，则接受请求
            if (request.currentFloor > status.currentFloor ||
                (request.currentFloor < status.currentFloor && isRequestUpward)) {
                return false;
            }
        }
    }
    
    return true;
}

void ElevatorController::moveElevator(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    auto elevator = m_elevators[elevatorId];
    
    // 如果没有目标楼层，不移动
    if (status.targetFloors.isEmpty()) {
        status.isMoving = false;
        status.direction = Direction::IDLE;
        return;
    }
    
    // 确定移动方向
    int nextTarget = status.targetFloors.first();
    if (nextTarget == status.currentFloor) {
        // 已到达目标楼层，处理到达事件
        handleElevatorArrival(elevatorId);
        return;
    }
    
    // 根据下一个目标楼层确定方向
    status.direction = nextTarget > status.currentFloor ? Direction::UP : Direction::DOWN;
    
    // 更新楼层
    if (status.direction == Direction::UP) {
        status.currentFloor++;
    } else {
        status.currentFloor--;
    }
    
    // 设置移动状态
    status.isMoving = true;
    
    // 更新电梯显示
    elevator->setCurrentFloor(status.currentFloor);
    elevator->setDirection(status.direction);
    
    // 检查是否需要在新楼层停靠
    if (shouldStopAtFloor(elevatorId, status.currentFloor)) {
        handleElevatorArrival(elevatorId);
    }
}

void ElevatorController::handleElevatorArrival(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    auto& requests = m_assignedRequests[elevatorId];
    
    // 先处理下客
    for (auto it = requests.begin(); it != requests.end();) {
        if (it->targetFloor == status.currentFloor) {
            // 乘客下电梯
            status.passengerCount -= it->passengerCount;
            updateStatistics(status.currentFloor, it->passengerCount);
            
            // 通知目标楼层更新状态（乘客到达）
            emit floorRequestCompleted(it->currentFloor, it->targetFloor, it->passengerCount);
            
            it = requests.erase(it);
        } else {
            ++it;
        }
    }
    
    // 再处理上客
    for (auto it = requests.begin(); it != requests.end();) {
        if (it->currentFloor == status.currentFloor) {
            // 检查是否还能容纳更多乘客
            if (status.passengerCount + it->passengerCount <= m_maxPassengers) {
                // 乘客上电梯
                status.passengerCount += it->passengerCount;
                emit floorRequestCompleted(it->currentFloor, it->targetFloor, -it->passengerCount);
                ++it;
            } else {
                // 电梯已满，等��下一趟
                break;
            }
        } else {
            ++it;
        }
    }
    
    // 更新电梯状态
    status.isMoving = false;
    status.lastStopTime = QDateTime::currentMSecsSinceEpoch();
    status.targetFloors.removeOne(status.currentFloor);
    
    // 如果没有更多目标楼层，设置为空闲状态
    if (status.targetFloors.isEmpty()) {
        status.direction = Direction::IDLE;
    } else {
        // 否则重新确定方向
        status.direction = determineDirection(elevatorId);
    }
    
    // 更新电梯显示
    m_elevators[elevatorId]->setCurrentFloor(status.currentFloor);
    m_elevators[elevatorId]->setDirection(status.direction);
    m_elevators[elevatorId]->setPassengerCount(status.passengerCount);
}

Direction ElevatorController::determineDirection(int elevatorId) const {
    const auto& status = m_elevatorStatus[elevatorId];
    if (status.targetFloors.isEmpty()) {
        return Direction::IDLE;
    }
    
    // 如果电梯已经有方向，检查是否需要继续保持该方向
    if (status.direction != Direction::IDLE) {
        bool hasTargetsInDirection = false;
        bool hasTargetsInOpposite = false;
        
        for (int floor : status.targetFloors) {
            if (status.direction == Direction::UP) {
                if (floor > status.currentFloor) {
                    hasTargetsInDirection = true;
                } else if (floor < status.currentFloor) {
                    hasTargetsInOpposite = true;
                }
            } else {
                if (floor < status.currentFloor) {
                    hasTargetsInDirection = true;
                } else if (floor > status.currentFloor) {
                    hasTargetsInOpposite = true;
                }
            }
        }
        
        // 如果当前方向还有目标楼层，继续保持方向
        if (hasTargetsInDirection) {
            return status.direction;
        }
        
        // 如果反方向有目标楼层，切换方向
        if (hasTargetsInOpposite) {
            return status.direction == Direction::UP ? Direction::DOWN : Direction::UP;
        }
    }
    
    // 如果是空闲状态或需要重新确定方向
    int nextTarget = status.targetFloors.first();
    if (nextTarget == status.currentFloor) {
        // 如果下一个目标是当前楼层，查看后续目标
        if (status.targetFloors.size() > 1) {
            nextTarget = status.targetFloors[1];
        }
    }
    
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
        
        // 清空该电梯的已分配请求
        auto& requests = m_assignedRequests[i];
        for (const auto& request : requests) {
            // 通知起始楼层清空请求
            emit floorRequestCompleted(request.currentFloor, request.targetFloor, -request.passengerCount);
        }
    }
    
    // 清空待处理队列中的请求
    while (!m_pendingRequests.isEmpty()) {
        const auto& request = m_pendingRequests.dequeue();
        // 通知起始楼层清空请求
        emit floorRequestCompleted(request.currentFloor, request.targetFloor, -request.passengerCount);
    }
    
    // 清空所有请求队列和统计数据
    m_pendingRequests.clear();
    m_assignedRequests.clear();
    m_floorStatistics.clear();
    
    // 通知所有楼层更新统计数据
    for (int floor = 1; floor <= 14; ++floor) {
        emit statisticsUpdated(floor, 0);
    }
}

bool ElevatorController::assignRequestToElevator(const PassengerRequest& request) {
    int bestElevator = findBestElevator(request);
    
    if (bestElevator >= 0) {
        auto& status = m_elevatorStatus[bestElevator];
        
        // 检查电梯是否已满
        if (status.passengerCount + request.passengerCount > m_maxPassengers) {
            return false;
        }
        
        // 添加请求到电梯的任务列表
        m_assignedRequests[bestElevator].append(request);
        
        // 按照电梯运行方向对目标楼层进行排序
        if (!status.targetFloors.contains(request.currentFloor)) {
            insertSortedFloor(bestElevator, request.currentFloor);
        }
        if (!status.targetFloors.contains(request.targetFloor)) {
            insertSortedFloor(bestElevator, request.targetFloor);
        }
        
        // 如果电梯是空闲的，根据新请求设置方向
        if (status.direction == Direction::IDLE) {
            if (request.currentFloor == status.currentFloor) {
                // 如果乘客在当前楼层，设置去往目标楼层的方向
                status.direction = request.targetFloor > status.currentFloor ? Direction::UP : Direction::DOWN;
            } else {
                // 否则，先设置去接乘客的方向
                status.direction = request.currentFloor > status.currentFloor ? Direction::UP : Direction::DOWN;
            }
        }
        
        return true;
    }
    
    return false;
}

void ElevatorController::insertSortedFloor(int elevatorId, int floor) {
    auto& status = m_elevatorStatus[elevatorId];
    auto& targetFloors = status.targetFloors;
    
    // 如果目标楼层列表为空，直接添加
    if (targetFloors.isEmpty()) {
        targetFloors.append(floor);
        return;
    }
    
    // 根据电梯运行方向对楼层进行排序
    if (status.direction == Direction::UP) {
        // 向上运行时，按照从小到大排序
        for (int i = 0; i < targetFloors.size(); ++i) {
            if (floor < targetFloors[i]) {
                targetFloors.insert(i, floor);
                return;
            }
        }
        targetFloors.append(floor);
    } else {
        // 向下运行时，按照从大到小排序
        for (int i = 0; i < targetFloors.size(); ++i) {
            if (floor > targetFloors[i]) {
                targetFloors.insert(i, floor);
                return;
            }
        }
        targetFloors.append(floor);
    }
}

void ElevatorController::generateRandomPassengers() {
    // 随机生成1-3个乘客请求
    int requestCount = QRandomGenerator::global()->bounded(1, 4);
    
    for (int i = 0; i < requestCount; ++i) {
        // 随机生成起始楼层和目标楼层
        int fromFloor = QRandomGenerator::global()->bounded(1, 15);
        int toFloor;
        do {
            toFloor = QRandomGenerator::global()->bounded(1, 15);
        } while (toFloor == fromFloor);

        // 随机生成1-5个乘客
        int passengerCount = QRandomGenerator::global()->bounded(1, 6);

        // 创建乘客请求
        PassengerRequest request;
        request.currentFloor = fromFloor;
        request.targetFloor = toFloor;
        request.passengerCount = passengerCount;
        request.timestamp = QDateTime::currentMSecsSinceEpoch();

        // 先发送信号通知楼层控件更新请求状态
        emit floorRequestAdded(fromFloor, toFloor, passengerCount);
        
        // 尝试立即分配给电梯
        if (!assignRequestToElevator(request)) {
            // 如果无法立即分配，则加入等待队列
            m_pendingRequests.enqueue(request);
        }

        // 更新楼层统计
        updateStatistics(fromFloor, passengerCount);
    }
}

void ElevatorController::processRequests() {
    // 处理所有待处理的请求
    while (!m_pendingRequests.isEmpty()) {
        PassengerRequest request = m_pendingRequests.head();
        
        // 尝试分配请求给最合适的电梯
        if (assignRequestToElevator(request)) {
            m_pendingRequests.dequeue();  // 成功分配后移除请求
        } else {
            break;  // 如果无法分配，等待下次更新
        }
    }
} 