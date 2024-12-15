#pragma once

#include "Types.h"
#include <vector>

class Elevator {
private:
    int currentFloor;
    int targetFloor;
    ElevatorState state;
    std::vector<Passenger> passengers;
    
public:
    void move();
    bool addPassenger(Passenger p);
    void updateState();
}; 