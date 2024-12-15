#pragma once
#include <string>

struct ElevatorConfig {
    static int FLOOR_COUNT;              // 楼层数
    static int ELEVATOR_COUNT;           // 电梯数量
    static int MAX_CAPACITY;             // 最大载客量
    static double FLOOR_TRAVEL_TIME;     // 层间运行时间
    static double MAX_IDLE_TIME;         // 最大空闲时间
    static double MAX_WAIT_TIME;         // 最大等待时间
    static int DEFAULT_REQUEST_COUNT;    // 默认请求数量
    
    static void loadDefaultConfig();
    static void saveConfig(const std::string& filename = "elevator.conf");
    static bool loadConfig(const std::string& filename = "elevator.conf");
}; 