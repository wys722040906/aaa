#include "visualizer.h"
#include "config.h"
#include <iostream>
#include <iomanip>

Visualizer::Visualizer() {
    initScreen();
}

void Visualizer::initScreen() {
    int height = (ElevatorConfig::FLOOR_COUNT * FLOOR_HEIGHT) + 1;
    int width = (ElevatorConfig::ELEVATOR_COUNT * ELEVATOR_WIDTH) + 10;
    screen.resize(height, std::vector<std::string>(width, " "));
}

void Visualizer::update(const Building& building) {
    initScreen();
    drawBuilding();
    drawElevators(building.getElevators());
    drawWaitingPassengers(building.getWaitingPassengers());
}

void Visualizer::render() const {
    std::cout << "\033[2J\033[H"; // 清屏
    
    for (const auto& row : screen) {
        for (const auto& cell : row) {
            std::cout << cell;
        }
        std::cout << std::endl;
    }
}

void Visualizer::drawBuilding() {
    // 绘制楼层标签和墙壁
    for (int floor = ElevatorConfig::FLOOR_COUNT; floor >= 1; --floor) {
        int y = (ElevatorConfig::FLOOR_COUNT - floor) * FLOOR_HEIGHT;
        
        // 绘制楼层标签
        screen[y][0] = getFloorLabel(floor);
        
        // 绘制墙壁和地板
        for (int x = 4; x < screen[y].size(); ++x) {
            screen[y][x] = FLOOR;
            if (x % ELEVATOR_WIDTH == 0) {
                for (int dy = 1; dy < FLOOR_HEIGHT; ++dy) {
                    screen[y + dy][x] = WALL;
                }
            }
        }
    }
}

void Visualizer::drawElevators(const std::vector<Elevator>& elevators) {
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        int x = 5 + (i * ELEVATOR_WIDTH);
        int y = (ElevatorConfig::FLOOR_COUNT - elevator.getCurrentFloor()) * FLOOR_HEIGHT;
        
        // 绘制电梯箱体
        screen[y][x] = ELEVATOR;
        screen[y][x+1] = ELEVATOR;
        
        // 绘制乘客数量
        std::string load = std::to_string(elevator.getCurrentLoad());
        screen[y][x+3] = load[0];
        if (load.length() > 1) {
            screen[y][x+4] = load[1];
        }
        
        // 绘制电梯状态指示器
        char stateChar;
        switch (elevator.getState()) {
            case ElevatorState::MOVING_UP: stateChar = '↑'; break;
            case ElevatorState::MOVING_DOWN: stateChar = '↓'; break;
            case ElevatorState::STOPPED: stateChar = '='; break;
            default: stateChar = '-';
        }
        screen[y-1][x+1] = stateChar;
    }
}

void Visualizer::drawWaitingPassengers(const std::vector<std::queue<Passenger>>& passengers) {
    for (int floor = 1; floor <= ElevatorConfig::FLOOR_COUNT; ++floor) {
        if (!passengers[floor].empty()) {
            int y = (ElevatorConfig::FLOOR_COUNT - floor) * FLOOR_HEIGHT;
            int count = passengers[floor].size();
            
            // 在楼层右侧显示等待人数
            std::string waitStr = "(" + std::to_string(count) + ")";
            for (size_t i = 0; i < waitStr.length(); ++i) {
                screen[y][screen[0].size() - waitStr.length() + i] = waitStr[i];
            }
            
            // 显示等待的乘客图标
            for (int i = 0; i < std::min(count, 5); ++i) {
                screen[y][screen[0].size() - waitStr.length() - 2 - i] = PASSENGER;
            }
        }
    }
}

std::string Visualizer::getFloorLabel(int floor) const {
    std::string label = std::to_string(floor);
    if (floor < 10) {
        label = " " + label;
    }
    return label + "F";
} 