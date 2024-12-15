#pragma once
#include <vector>
#include <queue>
#include "passenger.h"

enum class ElevatorState {
    IDLE,
    MOVING_UP,
    MOVING_DOWN,
    STOPPED
};

class Elevator {
private:
    int currentFloor;
    int capacity;
    std::vector<Passenger> passengers;
    ElevatorState state;
    int idleTime;
    static constexpr double FLOOR_TRAVEL_TIME = 5.0;  // 电梯运行每层楼需要5秒
    static constexpr double MAX_IDLE_TIME = 10.0;     // 最大空闲等待时间
    double floorTravelTime;                           // 当前层间运行时间计数器
    static constexpr double MAX_PASSENGER_WAIT_TIME = 120.0; // 乘客最大等待时间(T)为120秒
    double currentWaitTime;  // 当前等待时间计数器
    
public:
    Elevator(int capacity = 12);
    
    // 基本操作
    bool addPassenger(const Passenger& passenger);
    void removePassenger(int targetFloor);
    void move();
    void update(double deltaTime);
    
    // 获取状态
    int getCurrentFloor() const;
    int getCurrentLoad() const;
    ElevatorState getState() const;
    
    // 重置
    void reset();
    
    // 在Elevator类的public部分添加
    void setState(ElevatorState newState);
}; 