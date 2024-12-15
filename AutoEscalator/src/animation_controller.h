#pragma once
#include "building.h"
#include <string>
#include <vector>
#include <chrono>

class AnimationController {
private:
    static const int SCREEN_WIDTH = 80;
    static const int SCREEN_HEIGHT = 30;
    static const int ANIMATION_FRAME_RATE = 10; // 每秒帧数
    
    std::vector<std::string> screen;
    std::chrono::steady_clock::time_point lastFrameTime;
    double frameInterval;
    
    // 动画字符
    static const char ELEVATOR_CHAR = '■';
    static const char PASSENGER_CHAR = '●';
    static const char FLOOR_LINE = '─';
    static const char WALL = '│';
    
    void drawBuilding();
    void drawElevators(const std::vector<Elevator>& elevators);
    void drawPassengers(const Building& building);
    void drawFloorNumbers();
    void drawStatusBar(const Building& building);
    
public:
    AnimationController();
    
    void update(const Building& building);
    void render() const;
    bool shouldUpdateFrame() const;
    void setFrameRate(int fps);
}; 