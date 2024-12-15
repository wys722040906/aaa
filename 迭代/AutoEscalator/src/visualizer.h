#pragma once
#include "building.h"
#include <string>
#include <vector>

class Visualizer {
private:
    static const int FLOOR_HEIGHT = 3;  // 每层楼的显示高度
    static const int ELEVATOR_WIDTH = 6; // 每部电梯的显示宽度
    static const char WALL = '│';
    static const char FLOOR = '─';
    static const char ELEVATOR = '■';
    static const char PASSENGER = '●';
    
    std::vector<std::vector<std::string>> screen;
    
    void initScreen();
    void drawBuilding();
    void drawElevators(const std::vector<Elevator>& elevators);
    void drawWaitingPassengers(const std::vector<std::queue<Passenger>>& passengers);
    std::string getFloorLabel(int floor) const;
    
public:
    Visualizer();
    void update(const Building& building);
    void render() const;
}; 