#pragma once

#include <vector>
#include <map>
#include <string>

// 电梯状态枚举
enum class ElevatorState {
    IDLE,
    MOVING_UP,
    MOVING_DOWN,
    DOOR_OPENING,
    DOOR_CLOSING,
    MAINTENANCE,
    EMERGENCY
};

// 紧急情况类型
enum class EmergencyType {
    NONE,
    FIRE,
    DOOR_STUCK,
    OVERLOAD,
    POWER_OUT
};

// 乘客类
class Passenger {
public:
    int sourceFloor;
    int targetFloor;
    std::string id;
};

// 紧急情况类
class Emergency {
public:
    EmergencyType type;
    int elevatorId;
    std::string description;
}; 