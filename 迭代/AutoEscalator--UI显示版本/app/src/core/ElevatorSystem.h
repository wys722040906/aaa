#pragma once

#include "Elevator.h"
#include "PassengerManager.h"
#include "EmergencyHandler.h"
#include <vector>

class ElevatorSystem {
private:
    std::vector<Elevator> elevators;
    PassengerManager passengerManager;
    EmergencyHandler emergencyHandler;
    
public:
    void update();
    void handleRequests();
}; 