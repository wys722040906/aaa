#pragma once

#include "Types.h"
#include <vector>

class EmergencyHandler {
private:
    std::vector<Emergency> activeEmergencies;
    
public:
    void handleEmergency(EmergencyType type);
    void simulateRandomFailures();
}; 