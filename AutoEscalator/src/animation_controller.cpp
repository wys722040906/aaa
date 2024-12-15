#include "animation_controller.h"
#include <iostream>
#include <iomanip>
#include <thread>

AnimationController::AnimationController() 
    : frameInterval(1.0 / ANIMATION_FRAME_RATE) {
    screen.resize(SCREEN_HEIGHT);
    lastFrameTime = std::chrono::steady_clock::now();
}

void AnimationController::update(const Building& building) {
    // 清空屏幕缓冲
    for (auto& line : screen) {
        line = std::string(SCREEN_WIDTH, ' ');
    }
    
    // 绘制各个组件
    drawBuilding();
    drawElevators(building.getElevators());
    drawPassengers(building);
    drawFloorNumbers();
    drawStatusBar(building);
}

void AnimationController::render() const {
    std::cout << "\033[2J\033[H"; // 清屏并移动光标到左上角
    for (const auto& line : screen) {
        std::cout << line << std::endl;
    }
}

bool AnimationController::shouldUpdateFrame() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<double>(now - lastFrameTime).count();
    return duration >= frameInterval;
}

void AnimationController::setFrameRate(int fps) {
    frameInterval = 1.0 / fps;
}

void AnimationController::drawBuilding() {
    // 绘制楼层线
    for (int floor = 0; floor < 14; floor++) {
        int y = SCREEN_HEIGHT - 2 - floor * 2;
        for (int x = 10; x < SCREEN_WIDTH - 10; x++) {
            screen[y][x] = FLOOR_LINE;
        }
    }
    
    // 绘制电梯井道
    for (int x = 15; x < SCREEN_WIDTH - 15; x += 10) {
        for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {
            if (screen[y][x] != FLOOR_LINE) {
                screen[y][x] = WALL;
            }
        }
    }
}

void AnimationController::drawElevators(const std::vector<Elevator>& elevators) {
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        int x = 16 + i * 10;
        int y = SCREEN_HEIGHT - 2 - (elevator.getCurrentFloor() - 1) * 2;
        
        // 绘制电梯箱体
        screen[y][x] = ELEVATOR_CHAR;
        screen[y][x+1] = ELEVATOR_CHAR;
        
        // 绘制乘客数量
        std::string load = std::to_string(elevator.getCurrentLoad());
        screen[y][x+3] = load[0];
        if (load.length() > 1) {
            screen[y][x+4] = load[1];
        }
        
        // 绘制运动方向
        char dirChar = ' ';
        switch (elevator.getState()) {
            case ElevatorState::MOVING_UP: dirChar = '↑'; break;
            case ElevatorState::MOVING_DOWN: dirChar = '↓'; break;
            case ElevatorState::STOPPED: dirChar = '='; break;
            case ElevatorState::IDLE: dirChar = '-'; break;
        }
        screen[y-1][x+1] = dirChar;
    }
}

void AnimationController::drawPassengers(const Building& building) {
    const auto& waitingPassengers = building.getWaitingPassengers();
    for (int floor = 1; floor <= 14; ++floor) {
        if (!waitingPassengers[floor].empty()) {
            int y = SCREEN_HEIGHT - 2 - (floor - 1) * 2;
            int count = waitingPassengers[floor].size();
            
            // 最多显示5个等待乘客图标
            for (int i = 0; i < std::min(count, 5); ++i) {
                screen[y][5 + i] = PASSENGER_CHAR;
            }
            
            // 显示等待人数
            std::string waitStr = "(" + std::to_string(count) + ")";
            for (size_t i = 0; i < waitStr.length(); ++i) {
                screen[y][7 + i] = waitStr[i];
            }
        }
    }
}

void AnimationController::drawFloorNumbers() {
    for (int floor = 1; floor <= 14; ++floor) {
        int y = SCREEN_HEIGHT - 2 - (floor - 1) * 2;
        std::string floorNum = std::to_string(floor) + "F";
        screen[y][1] = floorNum[0];
        if (floorNum.length() > 1) {
            screen[y][2] = floorNum[1];
        }
        screen[y][3] = 'F';
    }
}

void AnimationController::drawStatusBar(const Building& building) {
    std::string status = "总等待人数: " + 
                        std::to_string(building.getTotalWaitingPassengers());
    screen[SCREEN_HEIGHT-1] = std::string(SCREEN_WIDTH, '─');
    int pos = (SCREEN_WIDTH - status.length()) / 2;
    screen[SCREEN_HEIGHT-1].replace(pos, status.length(), status);
} 