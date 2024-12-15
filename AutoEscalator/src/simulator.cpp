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
    
    performance.startMeasure("update_cycle");
    
    currentTime += deltaTime;
    
    // 检查是否到达上下班时间点
    performance.startMeasure("rush_hour_check");
    checkRushHour();
    performance.endMeasure("rush_hour_check");
    
    // 更新建筑物状态
    performance.startMeasure("building_update");
    building.update(deltaTime);
    performance.endMeasure("building_update");
    
    // 更新监控系统
    performance.startMeasure("monitor_update");
    monitor.update(currentTime);
    performance.endMeasure("monitor_update");
    
    // 显示当前状态
    performance.startMeasure("display_update");
    displayStatus();
    performance.endMeasure("display_update");
    
    performance.endMeasure("update_cycle");
    
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
    // 更新动画
    if (animator.shouldUpdateFrame()) {
        animator.update(building);
        animator.render();
    }
    
    // 更新其他显示
    monitorDisplay.update(building);
    monitorDisplay.render();
    
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

void Simulator::showPerformanceReport() const {
    std::cout << performance.getReport();
}

void Simulator::showEnergyReport() const {
    std::cout << building.getEnergyManager().getEnergyReport();
    std::cout << building.getEnergyManager().getEnergyOptimizationTips();
}

void Simulator::showMaintenanceMenu() {
    while (true) {
        std::cout << "\n=== 维护理系统 ===" << std::endl;
        std::cout << "1. 显示维护状态" << std::endl;
        std::cout << "2. 执行维护" << std::endl;
        std::cout << "3. 维修故障" << std::endl;
        std::cout << "4. 查看维护历史" << std::endl;
        std::cout << "5. 返回主菜单" << std::endl;
        
        char choice;
        std::cout << "\n请选择: ";
        std::cin >> choice;
        
        switch (choice) {
            case '1':
                displayMaintenanceStatus();
                break;
                
            case '2': {
                int elevatorId;
                std::cout << "请输入要维护的电梯编号(1-4): ";
                std::cin >> elevatorId;
                if (elevatorId >= 1 && elevatorId <= 4) {
                    building.getMaintenanceManager().performMaintenance(
                        elevatorId - 1, currentTime);
                    std::cout << "维护完成" << std::endl;
                } else {
                    std::cout << "无效的电梯编号" << std::endl;
                }
                break;
            }
            
            case '3': {
                int elevatorId;
                std::cout << "请输入要维修的电梯编号(1-4): ";
                std::cin >> elevatorId;
                if (elevatorId >= 1 && elevatorId <= 4) {
                    building.getMaintenanceManager().repairFault(
                        elevatorId - 1, currentTime);
                    std::cout << "维修完成" << std::endl;
                } else {
                    std::cout << "无效的电梯编号" << std::endl;
                }
                break;
            }
            
            case '4':
                displayMaintenanceHistory();
                break;
                
            case '5':
                return;
                
            default:
                std::cout << "无效选择" << std::endl;
        }
    }
}

void Simulator::displayMaintenanceStatus() const {
    std::cout << building.getMaintenanceManager().getMaintenanceReport();
}

void Simulator::displayMaintenanceHistory() const {
    const auto& history = building.getMaintenanceManager().getMaintenanceHistory();
    
    std::cout << "\n=== 维护历史记录 ===" << std::endl;
    if (history.empty()) {
        std::cout << "暂无维护记录" << std::endl;
        return;
    }
    
    for (const auto& record : history) {
        std::cout << "电梯 " << (record.elevatorId + 1) << " | ";
        std::cout << "类型: " << record.type << " | ";
        std::cout << "时间: " << formatTime(record.timestamp) << " | ";
        std::cout << "描述: " << record.description << std::endl;
    }
}

void Simulator::showDataAnalysisMenu() const {
    while (true) {
        std::cout << "\n=== 数据分析系统 ===" << std::endl;
        std::cout << "1. 显示运行报告" << std::endl;
        std::cout << "2. 显示负载图表" << std::endl;
        std::cout << "3. 显示等待时间分布" << std::endl;
        std::cout << "4. 显示能耗趋势" << std::endl;
        std::cout << "5. 导出数据到CSV" << std::endl;
        std::cout << "6. 返回主菜单" << std::endl;
        
        char choice;
        std::cout << "\n请选择: ";
        std::cin >> choice;
        
        const auto& recorder = building.getDataRecorder();
        
        switch (choice) {
            case '1':
                std::cout << recorder.generateReport();
                break;
                
            case '2':
                recorder.generateLoadGraph();
                break;
                
            case '3':
                recorder.generateWaitTimeGraph();
                break;
                
            case '4':
                recorder.generateEnergyGraph();
                break;
                
            case '5': {
                std::string filename;
                std::cout << "请输入导出文件名: ";
                std::cin >> filename;
                recorder.exportToCSV(filename);
                std::cout << "数据已导出到 " << filename << std::endl;
                break;
            }
            
            case '6':
                return;
                
            default:
                std::cout << "无效选择" << std::endl;
        }
    }
}

void Simulator::showHelp(const std::string& topic) const {
    helpSystem.showHelp(topic);
    
    // 如果是空主题，显示额外的快捷键信息
    if (topic.empty()) {
        std::cout << "\n=== 快捷键说明 ===" << std::endl;
        std::cout << "1-9: 对应菜单选项" << std::endl;
        std::cout << "H/h: 显示帮助" << std::endl;
        std::cout << "Q/q: 退出程序" << std::endl;
        std::cout << "R/r: 重置系统" << std::endl;
        std::cout << "S/s: 显示状态" << std::endl;
        
        std::cout << "\n=== 常用操作 ===" << std::endl;
        std::cout << "1. 开始模拟前，可以先调整系统配置" << std::endl;
        std::cout << "2. 可以通过文件导入或手动输入添加乘客请求" << std::endl;
        std::cout << "3. 运行过程中可以实时查看各种统计信息" << std::endl;
        std::cout << "4. 如果遇到问题，可以查看维护状态和警告信息" << std::endl;
    }
}

void Simulator::handleHelpCommand(const std::string& command) {
    if (command.empty()) {
        showHelp();
        return;
    }
    
    // 处理特定主题的帮助请求
    if (command == "config" || command == "设置") {
        showHelp("config");
    }
    else if (command == "elevator" || command == "电梯") {
        showHelp("elevator");
    }
    else if (command == "dispatch" || command == "调度") {
        showHelp("dispatch");
    }
    else if (command == "stats" || command == "统计") {
        showHelp("statistics");
    }
    else if (command == "maintenance" || command == "维护") {
        showHelp("maintenance");
    }
    else if (command == "energy" || command == "能源") {
        showHelp("energy");
    }
    else {
        // 尝试搜索相关主题
        auto results = helpSystem.searchHelp(command);
        if (results.empty()) {
            std::cout << "未找到相关帮助主题，请使用 'help' 查看所有主题" << std::endl;
        } else {
            std::cout << "找到以下相关主题：" << std::endl;
            for (const auto& result : results) {
                std::cout << "- " << result << std::endl;
            }
        }
    }
}