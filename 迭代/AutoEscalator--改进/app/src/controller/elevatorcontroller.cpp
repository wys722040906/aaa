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
    // 直接使用设置的高峰期乘客数量
    int requestCount = m_peakPassengerCount;
    
    // 根据高峰期类型设置目标楼层的倾向性
    QVector<int> preferredFloors;
    if (type == "上班") {
        // 上班时间倾向于去往办公楼(5-14层)
        for (int i = 5; i <= 14; ++i) {
            preferredFloors.append(i);
        }
    } else if (type == "午餐") {
        // 午餐时间倾向于去往餐厅楼层(1-4层)
        for (int i = 1; i <= 4; ++i) {
            preferredFloors.append(i);
        }
    } else if (type == "下班") {
        // 下班时间倾向于去往一楼
        preferredFloors.append(1);
    }
    
    for (int i = 0; i < requestCount; ++i) {
        int currentFloor, targetFloor;
        if (isGroundFloor) {
            // 从一楼上楼
            currentFloor = 1;
            if (!preferredFloors.isEmpty()) {
                // 80%概率选择倾向性楼层
                if (QRandomGenerator::global()->bounded(100) < 80) {
                    int index = QRandomGenerator::global()->bounded(preferredFloors.size());
                    targetFloor = preferredFloors[index];
                } else {
                    // 20%概率随机选择其他楼层
                    targetFloor = QRandomGenerator::global()->bounded(2, 15);
                }
            } else {
                targetFloor = QRandomGenerator::global()->bounded(2, 15);
            }
        } else {
            // 从其他楼层下楼
            if (type == "下班") {
                // 下班时从办公楼层(5-14层)下楼
                currentFloor = QRandomGenerator::global()->bounded(5, 15);
            } else {
                // 其他段从任意楼层
                currentFloor = QRandomGenerator::global()->bounded(2, 15);
            }
            
            if (!preferredFloors.isEmpty()) {
                // 80%概率选择倾向性楼层
                if (QRandomGenerator::global()->bounded(100) < 80) {
                    int index = QRandomGenerator::global()->bounded(preferredFloors.size());
                    targetFloor = preferredFloors[index];
                } else {
                    // 20%概率随机选择其他楼层
                    do {
                        targetFloor = QRandomGenerator::global()->bounded(1, 15);
                    } while (targetFloor == currentFloor);
                }
            } else {
                do {
                    targetFloor = QRandomGenerator::global()->bounded(1, 15);
                } while (targetFloor == currentFloor);
            }
        }
        
        // 生成较小的乘客组
        int passengerCount = QRandomGenerator::global()->bounded(1, 4);  // 1-3人一组
        addRequest(currentFloor, targetFloor, passengerCount);
    }
}

void ElevatorController::update() {
    // 更新每部��梯的状态
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
        // 检查是否有新标楼层
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
                // 已���在一楼，保持空闲状态
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
            // 检查是否需要改变原有路线
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
            // 电梯向下时：
            // 1. 请楼层在当前楼层之下
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
    
    // 如果没有标楼层，不移动
    if (status.targetFloors.isEmpty()) {
        status.isMoving = false;
        status.direction = Direction::IDLE;
        return;
    }
    
    // 确定移动方向
    int nextTarget = status.targetFloors.first();
    if (nextTarget == status.currentFloor) {
        // 已到达目标楼层，处理到达件
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
    if (elevatorId < 0 || elevatorId >= m_elevators.size()) {
        qDebug() << "Invalid elevator ID:" << elevatorId;
        return;
    }

    auto& status = m_elevatorStatus[elevatorId];
    
    // 开始开门过程
    m_currentElevatorId = elevatorId;
    
    // 断开之前的连接（如果有）
    disconnect(m_doorTimer, nullptr, this, nullptr);
    
    // 连接新的处理函数
    connect(m_doorTimer, &QTimer::timeout, this, [this]() {
        if (m_currentElevatorId >= 0 && m_currentElevatorId < m_elevators.size()) {
            processPassengersAtFloor(m_currentElevatorId);
            
            // 处理完成后更新电梯状态
            auto& status = m_elevatorStatus[m_currentElevatorId];
            status.isMoving = false;
            status.lastStopTime = QDateTime::currentMSecsSinceEpoch() / 1000;
            status.targetFloors.removeOne(status.currentFloor);
            
            // 重新确定电梯方向
            if (status.targetFloors.isEmpty()) {
                status.direction = Direction::IDLE;
            } else {
                status.direction = determineDirection(m_currentElevatorId);
            }
            
            // 更新电梯显示
            m_elevators[m_currentElevatorId]->setDirection(status.direction);
            
            // 重置当前处理的电梯ID
            m_currentElevatorId = -1;
        }
    });
    
    // 启动开关门计时器
    m_doorTimer->start(m_doorTime);
}

void ElevatorController::processPassengersAtFloor(int elevatorId) {
    auto& status = m_elevatorStatus[elevatorId];
    auto& requests = m_assignedRequests[elevatorId];
    
    // 先处理下客
    for (auto it = requests.begin(); it != requests.end();) {
        if (it->targetFloor == status.currentFloor) {
            // 乘客到达目标楼层
            status.passengerCount -= it->passengerCount;
            emit floorRequestCompleted(it->currentFloor, it->targetFloor, it->passengerCount);
            it = requests.erase(it);
        } else {
            ++it;
        }
    }
    
    // 再处理上客
    QVector<PassengerRequest> remainingRequests;
    int maxNewPassengers = m_maxPassengers - status.passengerCount;
    
    // 从等待队列中收集当前楼层的请求
    QVector<PassengerRequest> currentFloorRequests;
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end();) {
        if (it->currentFloor == status.currentFloor) {
            currentFloorRequests.append(*it);
            it = m_pendingRequests.erase(it);
        } else {
            ++it;
        }
    }
    
    // 处理收集到的请求
    for (const auto& request : currentFloorRequests) {
        if (maxNewPassengers <= 0) {
            // 电梯已满，剩余请求保持等待
            remainingRequests.append(request);
            continue;
        }
        
        // 检查方向是否匹配
        bool isRequestUpward = request.targetFloor > request.currentFloor;
        bool isElevatorUpward = status.direction == Direction::UP;
        
        if (status.direction != Direction::IDLE && isRequestUpward != isElevatorUpward) {
            // 方向不匹配，请求继续等待
            remainingRequests.append(request);
            continue;
        }
        
        // 检查是否可以完全接收这组乘客
        if (request.passengerCount <= maxNewPassengers) {
            // 全部上电梯
            status.passengerCount += request.passengerCount;
            maxNewPassengers -= request.passengerCount;
            
            // 更新楼层状态
            emit floorRequestCompleted(request.currentFloor, request.targetFloor, -request.passengerCount);
            
            // 添加目标楼层
            if (!status.targetFloors.contains(request.targetFloor)) {
                insertSortedFloor(elevatorId, request.targetFloor);
            }
            
            // 将请求添加到已分配列表
            m_assignedRequests[elevatorId].append(request);
        } else {
            // 部分上电梯
            int partialCount = maxNewPassengers;
            status.passengerCount += partialCount;
            
            // 更新楼层状态
            emit floorRequestCompleted(request.currentFloor, request.targetFloor, -partialCount);
            
            // 添加目标楼层
            if (!status.targetFloors.contains(request.targetFloor)) {
                insertSortedFloor(elevatorId, request.targetFloor);
            }
            
            // 剩余乘客继续等待
            PassengerRequest remainingRequest = request;
            remainingRequest.passengerCount -= partialCount;
            remainingRequests.append(remainingRequest);
            
            // 将部分请求添加到已分配列表
            PassengerRequest partialRequest = request;
            partialRequest.passengerCount = partialCount;
            m_assignedRequests[elevatorId].append(partialRequest);
            
            maxNewPassengers = 0;
        }
    }
    
    // 将未能上电梯的请求重新加入等待队列
    for (const auto& request : remainingRequests) {
        m_pendingRequests.enqueue(request);
    }
}

Direction ElevatorController::determineDirection(int elevatorId) const {
    const auto& status = m_elevatorStatus[elevatorId];
    if (status.targetFloors.isEmpty()) {
        return Direction::IDLE;
    }
    
    // 如果电梯已经有方向，检查是否需要继续保持���方向
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
        
        // 如果反方向有目标楼层切换方向
        if (hasTargetsInOpposite) {
            return status.direction == Direction::UP ? Direction::DOWN : Direction::UP;
        }
    }
    
    // 果空闲状态或需要重新确定方向
    int nextTarget = status.targetFloors.first();
    if (nextTarget == status.currentFloor) {
        // 如果下一个目标是当前楼层查看后续目标
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
    int maxRequestsPerElevator = 8;  // 每部电梯最多处理8个请求
    
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
        
        // 检查电梯是否已满
        auto& status = m_elevatorStatus[bestElevator];
        if (status.passengerCount + request.passengerCount > m_maxPassengers) {
            return false;
        }
        
        // 添加请求到电梯任务列表
        m_assignedRequests[bestElevator].append(request);
        
        // 按照电梯行方向对标楼层进行排序
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

    // 收集可用的楼层（当前请求数小于2的楼层）
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

    // 生成设定数量的请求
    for (int i = 0; i < m_randomPassengerCount && !availableFloors.isEmpty(); ++i) {
        // 随机选择一个起始楼层
        int randomIndex = QRandomGenerator::global()->bounded(availableFloors.size());
        int fromFloor = availableFloors[randomIndex];
        availableFloors.removeAt(randomIndex);  // 移除已使用的楼层
        
        // 选择目标楼层（避免同层）
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

        // 添加请求
        addRequest(fromFloor, toFloor, passengerCount);
    }
}

void ElevatorController::processRequests() {
    QQueue<PassengerRequest> unassignedRequests;
    
    // 处理所有待处理的请求
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
 