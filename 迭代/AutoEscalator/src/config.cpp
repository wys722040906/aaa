#include "config.h"
#include <fstream>
#include <iostream>

// 静态成员初始化
int ElevatorConfig::FLOOR_COUNT = 14;
int ElevatorConfig::ELEVATOR_COUNT = 4;
int ElevatorConfig::MAX_CAPACITY = 12;
double ElevatorConfig::FLOOR_TRAVEL_TIME = 5.0;
double ElevatorConfig::MAX_IDLE_TIME = 10.0;
double ElevatorConfig::MAX_WAIT_TIME = 120.0;
int ElevatorConfig::DEFAULT_REQUEST_COUNT = 5;

void ElevatorConfig::loadDefaultConfig() {
    FLOOR_COUNT = 14;
    ELEVATOR_COUNT = 4;
    MAX_CAPACITY = 12;
    FLOOR_TRAVEL_TIME = 5.0;
    MAX_IDLE_TIME = 10.0;
    MAX_WAIT_TIME = 120.0;
    DEFAULT_REQUEST_COUNT = 5;
}

void ElevatorConfig::saveConfig(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "无法保存配置文件" << std::endl;
        return;
    }
    
    file << "FLOOR_COUNT=" << FLOOR_COUNT << "\n"
         << "ELEVATOR_COUNT=" << ELEVATOR_COUNT << "\n"
         << "MAX_CAPACITY=" << MAX_CAPACITY << "\n"
         << "FLOOR_TRAVEL_TIME=" << FLOOR_TRAVEL_TIME << "\n"
         << "MAX_IDLE_TIME=" << MAX_IDLE_TIME << "\n"
         << "MAX_WAIT_TIME=" << MAX_WAIT_TIME << "\n"
         << "DEFAULT_REQUEST_COUNT=" << DEFAULT_REQUEST_COUNT << "\n";
    
    file.close();
}

bool ElevatorConfig::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "FLOOR_COUNT") FLOOR_COUNT = std::stoi(value);
        else if (key == "ELEVATOR_COUNT") ELEVATOR_COUNT = std::stoi(value);
        else if (key == "MAX_CAPACITY") MAX_CAPACITY = std::stoi(value);
        else if (key == "FLOOR_TRAVEL_TIME") FLOOR_TRAVEL_TIME = std::stod(value);
        else if (key == "MAX_IDLE_TIME") MAX_IDLE_TIME = std::stod(value);
        else if (key == "MAX_WAIT_TIME") MAX_WAIT_TIME = std::stod(value);
        else if (key == "DEFAULT_REQUEST_COUNT") DEFAULT_REQUEST_COUNT = std::stoi(value);
    }
    
    file.close();
    return true;
} 