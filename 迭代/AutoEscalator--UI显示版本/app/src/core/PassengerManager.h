#pragma once

#include "Types.h"
#include <map>
#include <vector>

class PassengerManager {
private:
    std::map<int, std::vector<Passenger>> floorPassengers;
    
public:
    void generatePassengers();
    void updatePassengerFlow();
}; 