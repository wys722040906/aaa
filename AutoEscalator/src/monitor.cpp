#include "monitor.h"
#include <iostream>
#include <iomanip>
#include "config.h"

Monitor::Monitor(const Building& b) : building(b) {}

void Monitor::update(double currentTime) {
    checkWaitingTimes(currentTime);
    checkElevatorLoads();
    checkQueueLengths();
}

void Monitor::addAlert(Alert::Level level, const std::string& message, double timestamp) {
    alerts.emplace_back(level, message, timestamp);
}

void Monitor::clearAlerts() {
    alerts.clear();
}

void Monitor::displayAlerts() const {
    if (alerts.empty()) {
        std::cout << "没有警告信息" << std::endl;
        return;
    }
    
    std::cout << "\n=== 系统警告 ===" << std::endl;
    for (const auto& alert : alerts) {
        std::string levelStr;
        switch (alert.level) {
            case Alert::Level::INFO: 
                std::cout << "\033[32m[信息]\033[0m "; // 绿色
                break;
            case Alert::Level::WARNING:
                std::cout << "\033[33m[警告]\033[0m "; // 黄色
                break;
            case Alert::Level::ERROR:
                std::cout << "\033[31m[错误]\033[0m "; // 红色
                break;
        }
        
        std::cout << "[" << std::fixed << std::setprecision(1) 
                 << alert.timestamp << "s] " 
                 << alert.message << std::endl;
    }
}

void Monitor::checkWaitingTimes(double currentTime) {
    const auto& waitingPassengers = building.getWaitingPassengers();
    for (int floor = 1; floor <= ElevatorConfig::FLOOR_COUNT; ++floor) {
        if (!waitingPassengers[floor].empty()) {
            const auto& passenger = waitingPassengers[floor].front();
            if (passenger.getWaitTime() >= MAX_WAIT_TIME_WARNING) {
                addAlert(Alert::Level::WARNING,
                        std::to_string(floor) + "楼的乘客等待时间过长",
                        currentTime);
            }
        }
    }
}

void Monitor::checkElevatorLoads() {
    const auto& elevators = building.getElevators();
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        double loadRatio = static_cast<double>(elevator.getCurrentLoad()) / 
                          ElevatorConfig::MAX_CAPACITY;
        
        if (loadRatio >= MAX_LOAD_RATIO) {
            addAlert(Alert::Level::WARNING,
                    "电梯 " + std::to_string(i + 1) + " 负载较高(" + 
                    std::to_string(static_cast<int>(loadRatio * 100)) + "%)",
                    0.0);
        }
    }
}

void Monitor::checkQueueLengths() {
    const auto& waitingPassengers = building.getWaitingPassengers();
    for (int floor = 1; floor <= ElevatorConfig::FLOOR_COUNT; ++floor) {
        if (waitingPassengers[floor].size() >= MAX_QUEUE_LENGTH) {
            addAlert(Alert::Level::WARNING,
                    std::to_string(floor) + "楼等待人数过多(" + 
                    std::to_string(waitingPassengers[floor].size()) + "人)",
                    0.0);
        }
    }
}

const std::vector<Monitor::Alert>& Monitor::getAlerts() const {
    return alerts;
} 