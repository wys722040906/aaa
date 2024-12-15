#include "core/PassengerManager.h"
#include <QTimer>
#include <QRandomGenerator>
#include <algorithm>

PassengerManager::PassengerManager(ElevatorDispatcher* dispatcher, QObject* parent)
    : QObject(parent), dispatcher(dispatcher) {
    peakHourTimer = new QTimer(this);
    connect(peakHourTimer, &QTimer::timeout, this, &PassengerManager::simulatePeakHour);
}

void PassengerManager::addPassenger(int fromFloor, int toFloor, Passenger::Type type) {
    auto passenger = std::make_shared<Passenger>(fromFloor, toFloor, type);
    floorInfo[fromFloor].waitingQueue.push(passenger);
    
    // 通知调度系统
    dispatcher->dispatchRequest(fromFloor, toFloor);
    
    // 更新统计信息
    dispatcher->updateFloorStatistics(fromFloor);
}

void PassengerManager::removePassenger(int floor) {
    auto& queue = floorInfo[floor].waitingQueue;
    if (!queue.empty()) {
        queue.pop();
    }
}

void PassengerManager::startPeakHourSimulation() {
    isPeakHour = true;
    dispatcher->setPeakHourMode(true);
    peakHourTimer->start(5000); // 每5秒生成一批乘客
}

void PassengerManager::stopPeakHourSimulation() {
    isPeakHour = false;
    dispatcher->setPeakHourMode(false);
    peakHourTimer->stop();
}

void PassengerManager::simulatePeakHour() {
    generateRandomPassengers();
}

void PassengerManager::generateRandomPassengers() {
    for (const auto& [floor, info] : floorInfo) {
        // 生成上班族
        if (info.workerCount > 0) {
            int count = QRandomGenerator::global()->bounded(1, info.workerCount + 1);
            for (int i = 0; i < count; ++i) {
                int toFloor = getRandomFloor();
                addPassenger(floor, toFloor, Passenger::Type::WORKER);
            }
        }
        
        // 生成老年人（较少频率）
        if (info.elderlyCount > 0 && QRandomGenerator::global()->bounded(100) < 20) {
            int count = QRandomGenerator::global()->bounded(1, info.elderlyCount + 1);
            for (int i = 0; i < count; ++i) {
                int toFloor = getRandomFloor();
                addPassenger(floor, toFloor, Passenger::Type::ELDERLY);
            }
        }
    }
}

int PassengerManager::getRandomFloor() const {
    std::vector<int> floors;
    for (const auto& [floor, _] : floorInfo) {
        floors.push_back(floor);
    }
    
    int index = QRandomGenerator::global()->bounded(static_cast<int>(floors.size()));
    return floors[index];
}

int PassengerManager::getWaitingCount(int floor) const {
    auto it = floorInfo.find(floor);
    if (it != floorInfo.end()) {
        return static_cast<int>(it->second.waitingQueue.size());
    }
    return 0;
}

std::vector<int> PassengerManager::getBusiestFloors() const {
    std::vector<std::pair<int, int>> floors;
    for (const auto& [floor, info] : floorInfo) {
        int totalCount = info.workerCount + info.elderlyCount;
        floors.emplace_back(floor, totalCount);
    }
    
    std::sort(floors.begin(), floors.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<int> busyFloors;
    for (size_t i = 0; i < std::min(size_t(3), floors.size()); ++i) {
        busyFloors.push_back(floors[i].first);
    }
    
    return busyFloors;
}

void PassengerManager::setFloorPopulation(int floor, int workerCount, int elderlyCount) {
    floorInfo[floor].workerCount = workerCount;
    floorInfo[floor].elderlyCount = elderlyCount;
}