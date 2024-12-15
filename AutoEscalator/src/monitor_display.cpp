#include "monitor_display.h"
#include <iostream>
#include <iomanip>

MonitorDisplay::MonitorDisplay() {
    screen.resize(SCREEN_HEIGHT);
    clear();
}

void MonitorDisplay::update(const Building& building) {
    clear();
    drawFrame();
    drawElevators(building.getElevators());
    drawStatistics(building);
    drawAlerts(building.getMaintenanceManager().getMaintenanceHistory());
    drawLegend();
}

void MonitorDisplay::render() const {
    std::cout << "\033[2J\033[H"; // 清屏
    for (const auto& line : screen) {
        std::cout << line << std::endl;
    }
}

void MonitorDisplay::clear() {
    for (auto& line : screen) {
        line = std::string(SCREEN_WIDTH, ' ');
    }
}

void MonitorDisplay::drawFrame() {
    // 绘制边框
    screen[0] = "┌" + std::string(SCREEN_WIDTH - 2, '─') + "┐";
    for (int i = 1; i < SCREEN_HEIGHT - 1; ++i) {
        screen[i][0] = '│';
        screen[i][SCREEN_WIDTH - 1] = '│';
    }
    screen[SCREEN_HEIGHT - 1] = "└" + std::string(SCREEN_WIDTH - 2, '─') + "┘";
    
    // 绘制标题
    std::string title = "电梯系统实时监控";
    int titlePos = (SCREEN_WIDTH - title.length()) / 2;
    screen[0].replace(titlePos, title.length(), title);
}

void MonitorDisplay::drawElevators(const std::vector<Elevator>& elevators) {
    const int ELEVATOR_DISPLAY_WIDTH = 15;
    const int START_X = 5;
    const int START_Y = 3;
    
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        int x = START_X + i * ELEVATOR_DISPLAY_WIDTH;
        int y = START_Y;
        
        // 绘制电梯框架
        std::string elevatorTitle = "电梯 " + std::to_string(i + 1);
        screen[y].replace(x, elevatorTitle.length(), elevatorTitle);
        
        // 绘制电梯状态
        std::string status = "楼层: " + std::to_string(elevator.getCurrentFloor());
        screen[y + 1].replace(x, status.length(), status);
        
        std::string load = "载客: " + std::to_string(elevator.getCurrentLoad()) + 
                          "/" + std::to_string(ElevatorConfig::MAX_CAPACITY);
        screen[y + 2].replace(x, load.length(), load);
        
        std::string state;
        switch (elevator.getState()) {
            case ElevatorState::MOVING_UP: 
                state = getColorCode("green") + "↑上行" + getColorCode("reset");
                break;
            case ElevatorState::MOVING_DOWN:
                state = getColorCode("green") + "↓下行" + getColorCode("reset");
                break;
            case ElevatorState::STOPPED:
                state = getColorCode("red") + "■停止" + getColorCode("reset");
                break;
            case ElevatorState::IDLE:
                state = getColorCode("yellow") + "□空闲" + getColorCode("reset");
                break;
        }
        screen[y + 3].replace(x, state.length(), state);
    }
}

void MonitorDisplay::drawStatistics(const Building& building) {
    const int START_X = 5;
    const int START_Y = 10;
    
    // 显示等待人数
    std::string waitingInfo = "总等待人数: " + 
                             std::to_string(building.getTotalWaitingPassengers());
    screen[START_Y].replace(START_X, waitingInfo.length(), waitingInfo);
    
    // 显示能源消耗
    std::string energyInfo = "总能耗: " + 
                            std::to_string(building.getEnergyManager().getTotalConsumption()) +
                            " kWh";
    screen[START_Y + 1].replace(START_X, energyInfo.length(), energyInfo);
}

void MonitorDisplay::drawAlerts(const std::vector<Monitor::Alert>& alerts) {
    const int START_X = 5;
    const int START_Y = 15;
    const int MAX_ALERTS = 5;
    
    screen[START_Y].replace(START_X, 7, "警告：");
    
    int alertCount = 0;
    for (auto it = alerts.rbegin(); 
         it != alerts.rend() && alertCount < MAX_ALERTS; 
         ++it, ++alertCount) {
        std::string alertText = "[" + std::to_string(alertCount + 1) + "] " + 
                               it->message;
        screen[START_Y + 1 + alertCount].replace(START_X, alertText.length(), alertText);
    }
}

void MonitorDisplay::drawLegend() {
    const int START_X = 5;
    const int START_Y = SCREEN_HEIGHT - 4;
    
    screen[START_Y].replace(START_X, 5, "图例:");
    screen[START_Y + 1].replace(START_X, 20, "↑↓ - 电梯运行方向");
    screen[START_Y + 2].replace(START_X, 20, "■  - 电梯停止");
    screen[START_Y + 3].replace(START_X, 20, "□  - 电梯空闲");
}

std::string MonitorDisplay::getColorCode(const std::string& color) {
    if (color == "red") return "\033[31m";
    if (color == "green") return "\033[32m";
    if (color == "yellow") return "\033[33m";
    if (color == "blue") return "\033[34m";
    if (color == "magenta") return "\033[35m";
    if (color == "cyan") return "\033[36m";
    if (color == "reset") return "\033[0m";
    return "";
}

std::string MonitorDisplay::centerText(const std::string& text, int width) {
    int padding = width - text.length();
    if (padding <= 0) return text;
    int leftPad = padding / 2;
    int rightPad = padding - leftPad;
    return std::string(leftPad, ' ') + text + std::string(rightPad, ' ');
} 