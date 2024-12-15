#pragma once
#include <vector>
#include <queue>
#include "elevator.h"
#include "passenger.h"
#include "dispatcher.h"

class Building {
private:
    static const int FLOOR_COUNT = 14;
    static const int ELEVATOR_COUNT = 4;
    
    std::vector<Elevator> elevators;
    std::vector<std::queue<Passenger>> waitingPassengers;
    Dispatcher dispatcher;
    
public:
    Building();
    
    void update(double deltaTime);
    void addRequest(int fromFloor, int toFloor, int passengerCount);
    void reset();
    
    // 获取状态
    const std::vector<Elevator>& getElevators() const;
    const std::vector<std::queue<Passenger>>& getWaitingPassengers() const;
    
    void displayWaitingPassengers() const;
    int getTotalWaitingPassengers() const;
    int getWaitingCountAtFloor(int floor) const;
    
    void setDispatchStrategy(Dispatcher::Strategy strategy);
    Dispatcher::Strategy getDispatchStrategy() const;
    const Dispatcher::Statistics& getDispatcherStatistics() const;
}; 