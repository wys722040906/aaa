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
    , m_randomPassengerCount(3)
    , m_peakPassengerCount(15)
    , m_isFirstRequest(true)
    , m_requestTimer(new QTimer(this))
    , m_doorTime(1000)  // 默认1秒
    , m_doorTimer(new QTimer(this))
    , m_currentElevatorId(-1)
    , m_simulationTime(0)  // 初始化模拟时间
    , m_dayDuration(240)   // 默认一天240秒
    , m_requestInterval(10)  // 默认10秒生成一次随机乘客
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

    // 初始化定时器
    m_doorTimer->setSingleShot(true);  // 设置为单次触发
    m_requestTimer->setSingleShot(false);  // 设置为重复触发
    
    // 初始化统计数据
    for (int floor = 1; floor <= 14; ++floor) {
        m_floorStatistics[floor] = 0;
    }
    
    // 初始化已分配请求容器
    m_assignedRequests.clear();
    for (int i = 0; i < m_elevators.size(); ++i) {
        m_assignedRequests[i] = QVector<PassengerRequest>();
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
    // 检查楼层范围
    if (currentFloor < 1 || currentFloor > 14 || targetFloor < 1 || targetFloor > 14) {
        return;
    }

    // 检查该楼层的当前请求数量（按请求数而不是乘客数计算）
    int currentFloorRequests = 0;
    for (const auto& request : m_pendingRequests) {
        if (request.currentFloor == currentFloor) {
            currentFloorRequests++;
        }
    }
    for (const auto& requests : m_assignedRequests) {
        for (const auto& request : requests) {
            if (request.currentFloor == currentFloor) {
                currentFloorRequests++;
            }
        }
    }

    // 如果该楼层请求数量已达到限制，不再添加新请求
    if (currentFloorRequests >= 2) {  // 每层最多2个请求
        return;
    }

    PassengerRequest request = {
        currentFloor,
        targetFloor,
        passengerCount,
        QDateTime::currentMSecsSinceEpoch()
    };
    
    // 尝试分配请求
    bool assigned = assignRequestToElevator(request);
    
    // 只有在成功分配或加入等待队列时才更新显示
    if (assigned || m_pendingRequests.size() < 15) {
        // 更新统计和显示
        m_floorStatistics[currentFloor] += passengerCount;
        emit statisticsUpdated(currentFloor, m_floorStatistics[currentFloor]);
        emit floorRequestAdded(currentFloor, targetFloor, passengerCount);
        
        // 如果分配失败，加入等待队列
        if (!assigned) {
            m_pendingRequests.enqueue(request);
        }
    }
}

void ElevatorController::addRushHourRequests(bool isGroundFloor, const QString& type) {
    // 空实现，不再处理高峰期请求
}

void ElevatorController::update() {
    static qint64 lastPrintTime = 0;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 更新模拟时间
    m_simulationTime++;
    
    // 每秒打印一次状态信息
    if (currentTime - lastPrintTime >= 1000) {
        printFloorRequests();
        lastPrintTime = currentTime;
    }
    
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
    
    // 如果电梯正在开关门，不进行其他操作
    if (m_currentElevatorId == elevatorId) {
        return;
    }
    
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
        
        // 计算基础成本：距离
        int distance = abs(status.currentFloor - request.currentFloor);
        
        // 计算负载成本：当前乘客数量（满载度越高，成本越高）
        int loadFactor = (status.passengerCount * 100) / m_maxPassengers;  // 计算满载分比
        int loadCost = loadFactor / 10;  // 每10%满载度增加1点成本
        
        // 计算方向兼容性成本
        int directionCost = 0;
        bool isRequestUpward = request.targetFloor > request.currentFloor;
        
        if (status.direction != Direction::IDLE) {
            bool isElevatorUpward = status.direction == Direction::UP;
            if (isRequestUpward == isElevatorUpward) {
                // 方向相同，优选择
                directionCost = 0;
            } else {
                // 方向相反，增加成本
                directionCost = 15;
            }
        } else {
            // 电梯，适当增加成本以平衡使用
            directionCost = 5;
        }
        
        // 计算任队列成本（队列越长，成本越高）
        int queueCost = m_assignedRequests[i].size() * 2;
        
        // 计算楼层顺路成本
        int detourCost = 0;
        if (!status.targetFloors.isEmpty()) {
            // 检查是否需要改变有路线
            int originalLastTarget = status.targetFloors.last();
            bool isOnTheWay = false;
            
            if (status.direction == Direction::UP) {
                isOnTheWay = request.currentFloor >= status.currentFloor && 
                            request.currentFloor <= originalLastTarget;
            } else if (status.direction == Direction::DOWN) {
                isOnTheWay = request.currentFloor <= status.currentFloor && 
                            request.currentFloor >= originalLastTarget;
            }
            
            if (!isOnTheWay) {
                detourCost = 10;  // 需要改变路线时增加成本
            }
        }
        
        // 计算总成本
        int totalCost = distance + loadCost + directionCost + queueCost + detourCost;
        
        // 如果电梯将满载，显著增加成本
        if (status.passengerCount + request.passengerCount > m_maxPassengers * 0.8) {
            totalCost += 20;
        }
        
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
    
    // 检查兼容性
    if (status.direction != Direction::IDLE) {
        bool isRequestUpward = request.targetFloor > request.currentFloor;
        bool isElevatorUpward = status.direction == Direction::UP;
        
        // 如果电梯正在移动，检查是否能顺路接客
        if (isElevatorUpward) {
            // 电梯向上时：
            // 1. 请求楼层在当前楼层之上
            // 2. 如果请求也是向上，则接受请求
            if (request.currentFloor < status.currentFloor ||
                (request.currentFloor > status.currentFloor && !isRequestUpward)) {
                return false;
            }
        } else {
            // 电梯向下：
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
    
    // 如果没有目标楼层，不移动
    if (status.targetFloors.isEmpty()) {
        status.isMoving = false;
        status.direction = Direction::IDLE;
        return;
    }
    
    // 确定移动方向
    if (status.direction == Direction::IDLE) {
        updateElevatorDirection(elevatorId);
    }
    
    // 根据方向更新当前楼层
    if (status.direction == Direction::UP) {
        status.currentFloor++;
    } else if (status.direction == Direction::DOWN) {
        status.currentFloor--;
    }
    
    // 设置移动状态
    status.isMoving = true;
    
    // 更新电梯显示
    m_elevators[elevatorId]->setCurrentFloor(status.currentFloor);
    qDebug() << QString("Elevator %1 moved to floor %2")
        .arg(elevatorId + 1)
        .arg(status.currentFloor);
}

void ElevatorController::handleElevatorArrival(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    
    // 暂停移动状态
    status.isMoving = false;
    
    // 记录停靠时间
    status.lastStopTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    
    // 开始开关门过程
    m_currentElevatorId = elevatorId;
    
    // 断开之前的连接（如果有）
    disconnect(m_doorTimer, nullptr, this, nullptr);
    
    // 连接新的处理函数
    connect(m_doorTimer, &QTimer::timeout, this, [this]() {
        if (m_currentElevatorId >= 0 && m_currentElevatorId < m_elevators.size()) {
            auto& status = m_elevatorStatus[m_currentElevatorId];
            
            // 处理乘客上下
            processPassengersAtFloor(m_currentElevatorId);
            
            // 移除当前楼层（如果是目标楼层）
            if (!status.targetFloors.isEmpty() && status.targetFloors.first() == status.currentFloor) {
                status.targetFloors.removeFirst();
            }
            
            // 更新电梯方向
            updateElevatorDirection(m_currentElevatorId);
            
            // 重置当前处理的电梯ID
            m_currentElevatorId = -1;
        }
    });
    
    // 启动开关门计时器
    m_doorTimer->start(m_doorTime);
}

void ElevatorController::updateElevatorDirection(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    
    if (status.targetFloors.isEmpty()) {
        status.direction = Direction::IDLE;
    } else {
        // 确定下一个目标楼层
        int nextTarget = status.targetFloors.first();
        
        // 根据下一个目标楼层确定方向
        if (nextTarget > status.currentFloor) {
            status.direction = Direction::UP;
        } else if (nextTarget < status.currentFloor) {
            status.direction = Direction::DOWN;
        }
        // 如果目标楼层就是当前楼层，保持当前方向不变
    }
    
    // 更新电梯显示
    m_elevators[elevatorId]->setDirection(status.direction);
}

bool ElevatorController::shouldStopAtFloor(int elevatorId, int floor) const {
    const auto& status = m_elevatorStatus[elevatorId];
    
    // 1. 检查是否是目标楼层
    if (status.targetFloors.contains(floor)) {
        return true;
    }
    
    // 2. 检查是否有匹配方向的乘客等待
    for (const auto& request : m_pendingRequests) {
        if (request.currentFloor == floor) {
            bool isRequestUpward = request.targetFloor > request.currentFloor;
            bool isElevatorUpward = status.direction == Direction::UP;
            
            // 如果电梯静止或方向匹配，则停靠
            if (status.direction == Direction::IDLE || isRequestUpward == isElevatorUpward) {
                // 还要检查电梯是否有空间
                if (status.passengerCount < m_maxPassengers) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void ElevatorController::processPassengersAtFloor(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    auto& requests = m_assignedRequests[elevatorId];
    
    // 收集当前楼层的所有请求变化，最后一次性更新状态
    QMap<int, int> floorRequestChanges;  // floor -> net change in requests
    
    // 1. 先处理下客
    for (auto it = requests.begin(); it != requests.end();) {
        if (it->targetFloor == status.currentFloor) {
            // 乘客到达目标楼层
            status.passengerCount -= it->passengerCount;
            
            // 记录请求变化
            floorRequestChanges[it->currentFloor] += it->passengerCount;
            
            // 移除完成的请求
            it = requests.erase(it);
        } else {
            ++it;
        }
    }
    
    // 2. 收集当前楼层的所有等待请求
    QVector<PassengerRequest> currentFloorRequests;
    QVector<PassengerRequest> remainingRequests;
    
    // 从等待队列中收集当前楼层的请求
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end();) {
        if (it->currentFloor == status.currentFloor) {
            // 检查方向是否匹配
            bool isRequestUpward = it->targetFloor > it->currentFloor;
            bool isElevatorUpward = status.direction == Direction::UP;
            
            // 电梯静止或方向匹配时可以搭乘
            if (status.direction == Direction::IDLE || isRequestUpward == isElevatorUpward) {
                currentFloorRequests.append(*it);
                it = m_pendingRequests.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
    
    // 3. 处理上客请求
    int availableSpace = m_maxPassengers - status.passengerCount;
    
    for (const auto& request : currentFloorRequests) {
        if (availableSpace <= 0) {
            // 电梯已满，剩余请求继续等待
            remainingRequests.append(request);
            continue;
        }
        
        // 确定实际可以上电梯的乘客数
        int actualPassengers = qMin(request.passengerCount, availableSpace);
        
        // 更新电梯状态
        status.passengerCount += actualPassengers;
        availableSpace -= actualPassengers;
        
        // 记录请求变化
        floorRequestChanges[request.currentFloor] -= actualPassengers;
        
        // 添加目标楼层
        if (!status.targetFloors.contains(request.targetFloor)) {
            insertSortedFloor(elevatorId, request.targetFloor);
        }
        
        // 如果还有剩余乘客，加入剩余请求
        if (actualPassengers < request.passengerCount) {
            PassengerRequest remainingRequest = request;
            remainingRequest.passengerCount -= actualPassengers;
            remainingRequests.append(remainingRequest);
        }
        
        // 将已接受的请求添加到电梯的任务列表
        PassengerRequest acceptedRequest = request;
        acceptedRequest.passengerCount = actualPassengers;
        m_assignedRequests[elevatorId].append(acceptedRequest);
    }
    
    // 4. 将未能上电梯的请求重新加入等待队列
    for (const auto& request : remainingRequests) {
        m_pendingRequests.enqueue(request);
    }
    
    // 5. 更新所有受影响楼层的状态
    for (auto it = floorRequestChanges.begin(); it != floorRequestChanges.end(); ++it) {
        int floor = it.key();
        
        // 检查该楼层是否还有其他请求
        bool hasRemainingRequests = false;
        
        // 检查待处理队列
        for (const auto& req : m_pendingRequests) {
            if (req.currentFloor == floor) {
                hasRemainingRequests = true;
                break;
            }
        }
        
        // 检查已分配请求
        if (!hasRemainingRequests) {
            for (const auto& reqs : m_assignedRequests) {
                for (const auto& req : reqs) {
                    if (req.currentFloor == floor) {
                        hasRemainingRequests = true;
                        break;
                    }
                }
                if (hasRemainingRequests) break;
            }
        }
        
        // 发送状态更新信号
        emit floorRequestCompleted(floor, 0, it.value());
        
        // 如果楼层没有剩余请求，发送状态重置信号
        if (!hasRemainingRequests) {
            emit floorRequestCompleted(floor, 0, 0);
        }
    }
}

Direction ElevatorController::determineDirection(int elevatorId) const {
    const auto& status = m_elevatorStatus[elevatorId];
    if (status.targetFloors.isEmpty()) {
        return Direction::IDLE;
    }
    
    // 如果电梯已经有方向，检查是否需要继续保持方向
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
        
        // 如果反方向有目标楼层切方向
        if (hasTargetsInOpposite) {
            return status.direction == Direction::UP ? Direction::DOWN : Direction::UP;
        }
    }
    
    // 果空闲状态或需要重新确定方向
    int nextTarget = status.targetFloors.first();
    if (nextTarget == status.currentFloor) {
        // 如下一个目标是当前楼层查看后续目标
        if (status.targetFloors.size() > 1) {
            nextTarget = status.targetFloors[1];
        }
    }
    
    return nextTarget > status.currentFloor ? Direction::UP : Direction::DOWN;
}

void ElevatorController::updateStatistics(int floor, int passengerCount) {
    m_floorStatistics[floor] += passengerCount;
    emit statisticsUpdated(floor, m_floorStatistics[floor]);
}

void ElevatorController::reset() {
    // 停止所有定时器
    m_doorTimer->stop();
    m_requestTimer->stop();
    
    // 清空所有请求队列
    m_pendingRequests.clear();
    m_assignedRequests.clear();
    
    // 重置电梯状态
    for (int i = 0; i < m_elevators.size(); ++i) {
        m_elevatorStatus[i] = {1, Direction::IDLE, 0, false, 0, QVector<int>()};
        m_elevators[i]->setCurrentFloor(1);
        m_elevators[i]->setDirection(Direction::IDLE);
        m_elevators[i]->setPassengerCount(0);
    }
    
    // 重置统计数据
    m_floorStatistics.clear();
    for (int floor = 1; floor <= 14; ++floor) {
        m_floorStatistics[floor] = 0;
        emit statisticsUpdated(floor, 0);
        emit floorRequestCompleted(floor, 0, 0);  // 清空所有楼层的请求
    }
    
    // 重置其他状态
    m_currentElevatorId = -1;
    m_isFirstRequest = true;
}

bool ElevatorController::assignRequestToElevator(const PassengerRequest& request) {
    // 检查单个电梯的请求队列长度限制
    int maxRequestsPerElevator = 8;  // 每电梯最多理8个请求
    
    // 检查总等待请求数限制
    int totalPendingRequests = m_pendingRequests.size();
    for (const auto& requests : m_assignedRequests) {
        totalPendingRequests += requests.size();
    }
    
    // 如果总请求数过多，暂停接受新请求
    if (totalPendingRequests >= 20) {
        return false;
    }
    
    int bestElevator = findBestElevator(request);
    
    if (bestElevator >= 0) {
        // 检查该电梯的请求队列是否已满
        if (m_assignedRequests[bestElevator].size() >= maxRequestsPerElevator) {
            return false;
        }
        
        // 检电梯是否已满
        auto& status = m_elevatorStatus[bestElevator];
        if (status.passengerCount + request.passengerCount > m_maxPassengers) {
            return false;
        }
        
        // 添加请求到电梯任务列表
        m_assignedRequests[bestElevator].append(request);
        
        // 对电梯行方向对标楼层进行排序
        if (!status.targetFloors.contains(request.currentFloor)) {
            insertSortedFloor(bestElevator, request.currentFloor);
        }
        if (!status.targetFloors.contains(request.targetFloor)) {
            insertSortedFloor(bestElevator, request.targetFloor);
        }
        
        return true;
    }
    
    return false;
}

void ElevatorController::insertSortedFloor(int elevatorId, int floor) {
    auto& status = m_elevatorStatus[elevatorId];
    auto& targetFloors = status.targetFloors;
    
    // 如果目标楼层表为空，直接添加
    if (targetFloors.isEmpty()) {
        targetFloors.append(floor);
        return;
    }
    
    // 据电梯运行方向对楼层行排序
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
    static qint64 lastGenerationTime = 0;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 确保请求生成有最小时间间隔（1秒）
    if (currentTime - lastGenerationTime < 1000) {
        return;
    }
    lastGenerationTime = currentTime;

    // 统计每层楼当前的请求数
    QMap<int, int> currentFloorRequests;
    for (int floor = 1; floor <= 14; ++floor) {
        currentFloorRequests[floor] = 0;
    }
    
    // 计算当前每层楼的请求数
    for (const auto& request : m_pendingRequests) {
        currentFloorRequests[request.currentFloor]++;
    }
    for (const auto& requests : m_assignedRequests) {
        for (const auto& request : requests) {
            currentFloorRequests[request.currentFloor]++;
        }
    }

    // 收集可用的楼层（当前请求数小于2楼层）
    QVector<int> availableFloors;
    for (int floor = 1; floor <= 14; ++floor) {
        if (currentFloorRequests[floor] < 2) {
            availableFloors.append(floor);
        }
    }
    
    // 如果没有可用楼层，放弃本次生成
    if (availableFloors.isEmpty()) {
        return;
    }

    // 生成设数量的请求
    for (int i = 0; i < m_randomPassengerCount && !availableFloors.isEmpty(); ++i) {
        // 随机选择一个起始楼层
        int randomIndex = QRandomGenerator::global()->bounded(availableFloors.size());
        int fromFloor = availableFloors[randomIndex];
        availableFloors.removeAt(randomIndex);  // 移除已使用的楼层
        
        // 目标楼层（避免同）
        QVector<int> possibleTargets;
        for (int floor = 1; floor <= 14; ++floor) {
            if (floor != fromFloor) {
                possibleTargets.append(floor);
            }
        }
        
        if (possibleTargets.isEmpty()) {
            continue;
        }

        int toFloor = possibleTargets[QRandomGenerator::global()->bounded(possibleTargets.size())];
        
        // 生成较小的乘客数量（1-3人）
        int passengerCount = QRandomGenerator::global()->bounded(1, 4);

        // 添加求
        addRequest(fromFloor, toFloor, passengerCount);
    }
}

void ElevatorController::processRequests() {
    QQueue<PassengerRequest> unassignedRequests;
    
    // ���理所有待处理的请求
    while (!m_pendingRequests.isEmpty()) {
        PassengerRequest request = m_pendingRequests.dequeue();
        
        // 尝试分配请求到最合适的电梯
        if (!assignRequestToElevator(request)) {
            // 如果无法分配，保存到临时队列
            unassignedRequests.enqueue(request);
        }
    }
    
    // 将未能分配的请求放回队列
    m_pendingRequests = unassignedRequests;
}

// 添加设置开关门时间的方法
void ElevatorController::setDoorTime(int milliseconds) {
    m_doorTime = milliseconds;
    qDebug() << "Door time updated to:" << milliseconds << "ms";
}

// 加新的私有方法来打印楼层请求信息
void ElevatorController::printFloorRequests() {
    // 只收集和显示状态，不进行任何状态更新
    QMap<int, QVector<QPair<int, int>>> floorRequests;
    
    // 收集所有楼层的实际请求
    for (const auto& request : m_pendingRequests) {
        floorRequests[request.currentFloor].append({request.targetFloor, request.passengerCount});
    }
    
    for (const auto& requests : m_assignedRequests) {
        for (const auto& request : requests) {
            floorRequests[request.currentFloor].append({request.targetFloor, request.passengerCount});
        }
    }

    // 打印系统状态
    qDebug() << "\n=== 系统状态信息 ===";
    
    // 打印时间信息
    int daySeconds = m_simulationTime % m_dayDuration;
    int hour = (daySeconds * 24) / m_dayDuration;
    int minute = ((daySeconds * 24 * 60) / m_dayDuration) % 60;
    qDebug().noquote() << QString("当前时间: %1:%2")
        .arg(hour, 2, 10, QChar('0'))
        .arg(minute, 2, 10, QChar('0'));
    
    qDebug().noquote() << QString("随机乘客生成间隔: %1秒").arg(m_requestInterval);
    
    // 打印电梯状态
    qDebug() << "\n--- 电梯状态 ---";
    for (int i = 0; i < m_elevators.size(); ++i) {
        const auto& status = m_elevatorStatus[i];
        QString dirStr;
        switch (status.direction) {
            case Direction::UP: dirStr = "↑"; break;
            case Direction::DOWN: dirStr = "↓"; break;
            default: dirStr = "•"; break;
        }
        
        // 修复这里的转换问题
        QString targetFloorsStr;
        if (status.targetFloors.isEmpty()) {
            targetFloorsStr = "无";
        } else {
            QStringList floors;
            for (int floor : status.targetFloors) {
                floors << QString::number(floor);
            }
            targetFloorsStr = floors.join(",");
        }
        
        qDebug().noquote() << QString("电梯%1: %2楼%3 载客:%4人 目标楼层:[%5]")
            .arg(i + 1)
            .arg(status.currentFloor, 2)
            .arg(dirStr)
            .arg(status.passengerCount, 2)
            .arg(targetFloorsStr);
    }
    
    // 打印楼层请求状态
    qDebug() << "\n--- 楼层请求状态 ---";
    for (int floor = 14; floor >= 1; --floor) {
        QString floorInfo = QString("%1楼:").arg(floor, 2);
        
        if (floorRequests.contains(floor) && !floorRequests[floor].isEmpty()) {
            QStringList requestInfos;
            for (const auto& request : floorRequests[floor]) {
                requestInfos << QString("到%1楼(%2人)").arg(request.first).arg(request.second);
            }
            floorInfo += " " + requestInfos.join(", ");
        } else {
            floorInfo += " 无请求";
        }
        
        qDebug().noquote() << floorInfo;
    }
    
    qDebug() << "\n待处理请求数:" << m_pendingRequests.size();
    qDebug() << "==================\n";
}
 