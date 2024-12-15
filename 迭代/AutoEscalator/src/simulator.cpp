#include "simulator.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <iomanip>

Simulator::Simulator() 
    : currentTime(0.0), totalTime(24.0 * 3600), isRunning(false),
      monitor(building) {} // 24小时转换为秒

void Simulator::start() {
    isRunning = true;
    currentTime = 0.0;
    stats.reset();
    building.reset();
    std::cout << "模拟开始..." << std::endl;
}

void Simulator::reset() {
    isRunning = false;
    currentTime = 0.0;
    stats.reset();
    building.reset();
    std::cout << "系统已重置" << std::endl;
}

void Simulator::update(double deltaTime) {
    if (!isRunning) return;
    
    currentTime += deltaTime;
    
    // 检查是否到达上下班时间点
    checkRushHour();
    
    // 更新建筑物状态
    building.update(deltaTime);
    
    // 更新监控系统
    monitor.update(currentTime);
    
    // 显示当前状态
    displayStatus();
    
    // 检查是否结束模拟
    if (currentTime >= totalTime) {
        endSimulation();
    }
}

void Simulator::generateRandomRequests() {
    const int DEFAULT_REQUEST_COUNT = 5;
    
    for (int i = 0; i < DEFAULT_REQUEST_COUNT; ++i) {
        int fromFloor = 1; // 上班高峰期从1楼出发
        int toFloor = Utils::generateRandomNumber(2, 14);
        int passengerCount = Utils::generateRandomNumber(1, 5);
        
        building.addRequest(fromFloor, toFloor, passengerCount);
        stats.recordPassenger(fromFloor, toFloor);
    }
}

void Simulator::loadRequestsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "无法打开文件: " << filename << std::endl;
        return;
    }
    
    int fromFloor, toFloor, passengerCount;
    while (file >> fromFloor >> toFloor >> passengerCount) {
        building.addRequest(fromFloor, toFloor, passengerCount);
        stats.recordPassenger(fromFloor, toFloor);
    }
    
    file.close();
}

bool Simulator::isSimulationRunning() const {
    return isRunning;
}

double Simulator::getCurrentTime() const {
    return currentTime;
}

void Simulator::checkRushHour() {
    double dayProgress = currentTime / totalTime;
    
    // 上班高峰期（2/10和6/10时刻）
    if (isNearTime(dayProgress, 0.2) || isNearTime(dayProgress, 0.6)) {
        generateRandomRequests();
    }
    // 下班高峰期（4/10和8/10时刻）
    else if (isNearTime(dayProgress, 0.4) || isNearTime(dayProgress, 0.8)) {
        generateDownwardRequests();
    }
}

bool Simulator::isNearTime(double current, double target) {
    const double THRESHOLD = 0.01; // 1%的时间窗口
    return std::abs(current - target) < THRESHOLD;
}

void Simulator::generateDownwardRequests() {
    const int DEFAULT_REQUEST_COUNT = 5;
    
    for (int i = 0; i < DEFAULT_REQUEST_COUNT; ++i) {
        int fromFloor = Utils::generateRandomNumber(2, 14);
        int toFloor = 1; // 下班高峰期到1楼
        int passengerCount = Utils::generateRandomNumber(1, 5);
        
        building.addRequest(fromFloor, toFloor, passengerCount);
        stats.recordPassenger(fromFloor, toFloor);
    }
}

void Simulator::displayStatus() {
    // 更新并显示可视化界面
    visualizer.update(building);
    visualizer.render();
    
    // 显示系统信息
    std::cout << "\n=== 系统信息 ===" << std::endl;
    std::cout << "当前时间: " << formatTime(currentTime) << std::endl;
    std::cout << "总等待人数: " << building.getTotalWaitingPassengers() << std::endl;
    
    // 显示电梯详细信息
    std::cout << "\n=== 电梯详细信息 ===" << std::endl;
    const auto& elevators = building.getElevators();
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        std::cout << "电梯 " << (i + 1) << ": ";
        std::cout << "楼层=" << elevator.getCurrentFloor() << " ";
        std::cout << "载客=" << elevator.getCurrentLoad() << "/" 
                 << ElevatorConfig::MAX_CAPACITY << " ";
        std::cout << "状态=" << getStateString(elevator.getState()) << std::endl;
    }
    
    // 显示警告信息
    monitor.displayAlerts();
}

void Simulator::endSimulation() {
    isRunning = false;
    std::cout << "\n模拟结束！" << std::endl;
    stats.displayChart();
}

std::string Simulator::formatTime(double seconds) {
    int hours = static_cast<int>(seconds / 3600) % 24;
    int minutes = static_cast<int>(seconds / 60) % 60;
    int secs = static_cast<int>(seconds) % 60;
    
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, secs);
    return std::string(buffer);
}

std::string Simulator::getStateString(ElevatorState state) {
    switch (state) {
        case ElevatorState::IDLE: return "空闲";
        case ElevatorState::MOVING_UP: return "上行";
        case ElevatorState::MOVING_DOWN: return "下行";
        case ElevatorState::STOPPED: return "停止";
        default: return "未知";
    }
}

void Simulator::handleManualRequest() {
    int fromFloor, toFloor, passengerCount;
    
    std::cout << "请输入乘客请求（起始楼层 目标楼层 乘客数）: ";
    std::cin >> fromFloor >> toFloor >> passengerCount;
    
    if (fromFloor >= 1 && fromFloor <= 14 && 
        toFloor >= 1 && toFloor <= 14 && 
        fromFloor != toFloor && 
        passengerCount > 0 && passengerCount <= 12) {
        
        building.addRequest(fromFloor, toFloor, passengerCount);
        stats.recordPassenger(fromFloor, toFloor);
        std::cout << "请求已添加" << std::endl;
    } else {
        std::cout << "无效的请求参数" << std::endl;
    }
}

void Simulator::showConfigMenu() {
    while (true) {
        std::cout << "\n=== 系统配置 ===" << std::endl;
        std::cout << "1. 调整电梯运行速度 (当前: " << ElevatorConfig::FLOOR_TRAVEL_TIME << "秒/层)" << std::endl;
        std::cout << "2. 调整最大空闲时间 (当前: " << ElevatorConfig::MAX_IDLE_TIME << "秒)" << std::endl;
        std::cout << "3. 调整最大等待时间 (当前: " << ElevatorConfig::MAX_WAIT_TIME << "秒)" << std::endl;
        std::cout << "4. 调整电梯容量 (当前: " << ElevatorConfig::MAX_CAPACITY << "人)" << std::endl;
        std::cout << "5. 调整默认请求数量 (当前: " << ElevatorConfig::DEFAULT_REQUEST_COUNT << "个)" << std::endl;
        std::cout << "6. 保存配置" << std::endl;
        std::cout << "7. 加载配置" << std::endl;
        std::cout << "8. 恢复默认设置" << std::endl;
        std::cout << "9. 返回主菜单" << std::endl;
        
        char choice;
        std::cout << "\n请选择: ";
        std::cin >> choice;
        
        double newValue;
        switch (choice) {
            case '1':
                std::cout << "请输入新的运行速度(秒/层): ";
                std::cin >> newValue;
                if (newValue > 0) ElevatorConfig::FLOOR_TRAVEL_TIME = newValue;
                break;
                
            case '2':
                std::cout << "请输入新的最大空闲时间(秒): ";
                std::cin >> newValue;
                if (newValue > 0) ElevatorConfig::MAX_IDLE_TIME = newValue;
                break;
                
            case '3':
                std::cout << "请输入新的最大等待时间(秒): ";
                std::cin >> newValue;
                if (newValue > 0) ElevatorConfig::MAX_WAIT_TIME = newValue;
                break;
                
            case '4':
                int newCapacity;
                std::cout << "请输入新的电梯容量: ";
                std::cin >> newCapacity;
                if (newCapacity > 0) ElevatorConfig::MAX_CAPACITY = newCapacity;
                break;
                
            case '5':
                int newCount;
                std::cout << "请输入新的默认请求数量: ";
                std::cin >> newCount;
                if (newCount > 0) ElevatorConfig::DEFAULT_REQUEST_COUNT = newCount;
                break;
                
            case '6':
                ElevatorConfig::saveConfig();
                std::cout << "配置已保存" << std::endl;
                break;
                
            case '7':
                if (ElevatorConfig::loadConfig()) {
                    std::cout << "配置已加载" << std::endl;
                } else {
                    std::cout << "加载配置失败" << std::endl;
                }
                break;
                
            case '8':
                ElevatorConfig::loadDefaultConfig();
                std::cout << "已恢复默认设置" << std::endl;
                break;
                
            case '9':
                return;
                
            default:
                std::cout << "无效选择" << std::endl;
        }
    }
}

void Simulator::showDispatcherMenu() {
    while (true) {
        std::cout << "\n=== 调度策略管理 ===" << std::endl;
        std::cout << "当前策略: " 
                 << Dispatcher::getStrategyName(building.getDispatchStrategy()) 
                 << std::endl;
        std::cout << "1. 切换到最近优先策略" << std::endl;
        std::cout << "2. 切换到负载均衡策略" << std::endl;
        std::cout << "3. 切换到节能模式策略" << std::endl;
        std::cout << "4. 显示调度统计信息" << std::endl;
        std::cout << "5. 返回主菜单" << std::endl;
        
        char choice;
        std::cout << "\n请选择: ";
        std::cin >> choice;
        
        switch (choice) {
            case '1':
                building.setDispatchStrategy(Dispatcher::Strategy::NEAREST_FIRST);
                std::cout << "已切换到最近优先策略" << std::endl;
                break;
                
            case '2':
                building.setDispatchStrategy(Dispatcher::Strategy::LOAD_BALANCED);
                std::cout << "已切换到负载均衡策略" << std::endl;
                break;
                
            case '3':
                building.setDispatchStrategy(Dispatcher::Strategy::ENERGY_SAVING);
                std::cout << "已切换到节能模式策略" << std::endl;
                break;
                
            case '4':
                displayDispatcherStatistics();
                break;
                
            case '5':
                return;
                
            default:
                std::cout << "无效选择" << std::endl;
        }
    }
}

void Simulator::displayDispatcherStatistics() const {
    const auto& stats = building.getDispatcherStatistics();
    
    std::cout << "\n=== 调度统计信息 ===" << std::endl;
    std::cout << "总分配请求数: " << stats.totalAssignments << std::endl;
    std::cout << "成功分配数: " << stats.successfulAssignments << std::endl;
    
    if (stats.successfulAssignments > 0) {
        std::cout << "平均等待时间: " << std::fixed << std::setprecision(2) 
                 << stats.averageWaitTime << "秒" << std::endl;
        std::cout << "平均移动距离: " << std::fixed << std::setprecision(2) 
                 << stats.averageDistance << "层" << std::endl;
        std::cout << "分配成功率: " << std::fixed << std::setprecision(1)
                 << (stats.successfulAssignments * 100.0 / stats.totalAssignments)
                 << "%" << std::endl;
    }
} 