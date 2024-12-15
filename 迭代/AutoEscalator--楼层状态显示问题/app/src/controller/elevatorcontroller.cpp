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
    // 检查楼层范围
    if (currentFloor < 1 || currentFloor > 14 || targetFloor < 1 || targetFloor > 14) {
        return;
    }

    // 检查是否已经存在相同的请求
    for (const auto& request : m_pendingRequests) {
        if (request.currentFloor == currentFloor && 
            request.targetFloor == targetFloor) {
            return;
        }
    }
    
    // 检查已分配的请求
    for (const auto& requests : m_assignedRequests) {
        for (const auto& request : requests) {
            if (request.currentFloor == currentFloor && 
                request.targetFloor == targetFloor) {
                return;
            }
        }
    }

    // 检查该楼层的当前请求数量
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
    if (currentFloorRequests >= 2) {
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
    
    // 只有在成功分配或等待队列未满时才更新统计和发送信号
    if (assigned || m_pendingRequests.size() < 15) {
        m_floorStatistics[currentFloor] += passengerCount;
        emit statisticsUpdated(currentFloor, m_floorStatistics[currentFloor]);
        emit floorRequestAdded(currentFloor, targetFloor, passengerCount);
        
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
        // 上班时间倾向于去往办公楼层(5-14层)
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
                // 其他时段从任意楼层
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
        
        // 计算基础成本：距离
        int distance = abs(status.currentFloor - request.currentFloor);
        
        // 计算负载成本：当前乘客数量（满载度越高，成本越高）
        int loadFactor = (status.passengerCount * 100) / m_maxPassengers;  // 计算满载百分比
        int loadCost = loadFactor / 10;  // 每10%满载度增加1点成本
        
        // 计算方向兼容性成本
        int directionCost = 0;
        bool isRequestUpward = request.targetFloor > request.currentFloor;
        
        if (status.direction != Direction::IDLE) {
            bool isElevatorUpward = status.direction == Direction::UP;
            if (isRequestUpward == isElevatorUpward) {
                // 方向相同，优先选择
                directionCost = 0;
            } else {
                // 方向相反，增加成本
                directionCost = 15;
            }
        } else {
            // 电梯���，适当增加成本以平衡使用
            directionCost = 5;
        }
        
        // 计算任务队列成本（队列越长，成本越高）
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
        
        // 如果电梯即将满载，显著增加成本
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
    
    // 如果没有标楼层，不移动
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
            // 乘客到达目标楼层
            status.passengerCount -= it->passengerCount;
            emit floorRequestCompleted(it->currentFloor, it->targetFloor, it->passengerCount);
            it = requests.erase(it);
        } else {
            ++it;
        }
    }
    
    // 再处理上客，但要考虑电梯容量和方向
    int maxNewPassengers = m_maxPassengers - status.passengerCount;
    for (auto it = requests.begin(); it != requests.end() && maxNewPassengers > 0;) {
        if (it->currentFloor == status.currentFloor) {
            bool isRequestUpward = it->targetFloor > it->currentFloor;
            bool isElevatorUpward = status.direction == Direction::UP;
            
            if (status.direction == Direction::IDLE || isRequestUpward == isElevatorUpward) {
                if (it->passengerCount <= maxNewPassengers) {
                    // 乘客上电梯前，更新楼层统计
                    m_floorStatistics[it->currentFloor] -= it->passengerCount;
                    emit statisticsUpdated(it->currentFloor, m_floorStatistics[it->currentFloor]);
                    
                    // 乘客上电梯
                    status.passengerCount += it->passengerCount;
                    maxNewPassengers -= it->passengerCount;
                    emit floorRequestCompleted(it->currentFloor, it->targetFloor, -it->passengerCount);
                    
                    it = requests.erase(it);
                } else {
                    break;  // 电梯已满
                }
            } else {
                ++it;  // 方向不匹配
            }
        } else {
            ++it;
        }
    }
    
    // 更新电梯状态
    status.isMoving = false;
    status.lastStopTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    status.targetFloors.removeOne(status.currentFloor);
    
    // 重新确定电梯方向
    if (status.targetFloors.isEmpty()) {
        status.direction = Direction::IDLE;
    } else {
        status.direction = determineDirection(elevatorId);
    }
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
    
    // 重置所有请求队列和统计数据
    m_pendingRequests.clear();
    m_assignedRequests.clear();
    m_floorStatistics.clear();
    
    // 通知所有楼层更新统计数据
    for (int floor = 1; floor <= 14; ++floor) {
        emit statisticsUpdated(floor, 0);
    }
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
        
        // 按照电梯行方向对目标楼层进行排序
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
    
    // 根据电梯运行方向对楼层行排序
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

    // 检查是否是第一次调用
    if (lastGenerationTime == 0) {
        lastGenerationTime = currentTime;
        return;
    }
    lastGenerationTime = currentTime;

    // 限制同时等待的请求总数
    int totalPendingRequests = m_pendingRequests.size();
    for (const auto& requests : m_assignedRequests) {
        totalPendingRequests += requests.size();
    }
    
    // 如果等待的请求太多，暂停生成新请求
    if (totalPendingRequests >= 15) {
        qDebug() << "Too many pending requests, skipping generation";
        return;
    }

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

    // 生成新的请求（限制每次最多生成1个请求）
    int requestCount = qMin(1, m_randomPassengerCount);
    
    // 收集所有可用的楼层（当前请求数小于2的楼层）
    QVector<int> availableFloors;
    for (int floor = 1; floor <= 14; ++floor) {
        if (currentFloorRequests[floor] < 2) {
            availableFloors.append(floor);
        }
    }
    
    // 如果没有可用楼层，放弃本次生成
    if (availableFloors.isEmpty()) {
        qDebug() << "No available floors for new requests";
        return;
    }

    // 随机选择一个起始楼层
    int fromFloor = availableFloors[QRandomGenerator::global()->bounded(availableFloors.size())];
    
    // 选择目标楼层（避免同层）
    QVector<int> possibleTargets;
    for (int floor = 1; floor <= 14; ++floor) {
        if (floor != fromFloor) {
            possibleTargets.append(floor);
        }
    }
    
    if (possibleTargets.isEmpty()) {
        qDebug() << "No available target floors";
        return;
    }

    int toFloor = possibleTargets[QRandomGenerator::global()->bounded(possibleTargets.size())];
    
    // 生成较小的乘客数量（1-3人）
    int passengerCount = QRandomGenerator::global()->bounded(1, 4);

    // 添加请求前再次验证
    if (currentFloorRequests[fromFloor] < 2) {
        qDebug() << "Generating request:" << fromFloor << "->" << toFloor 
                 << "with" << passengerCount << "passengers";
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