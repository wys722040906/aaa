#pragma once
#include "building.h"
#include <string>
#include <vector>

class MonitorDisplay {
private:
    static const int SCREEN_WIDTH = 100;
    static const int SCREEN_HEIGHT = 30;
    std::vector<std::string> screen;
    
    void drawFrame();
    void drawElevators(const std::vector<Elevator>& elevators);
    void drawStatistics(const Building& building);
    void drawAlerts(const std::vector<Monitor::Alert>& alerts);
    void drawLegend();
    
public:
    MonitorDisplay();
    
    void update(const Building& building);
    void render() const;
    void clear();
    
    // 辅助方法
    static std::string getColorCode(const std::string& color);
    static std::string centerText(const std::string& text, int width);
}; 