#pragma once
#include "config.h"

class Passenger {
private:
    int sourceFloor;
    int targetFloor;
    double waitTime;
    bool isTimeout;
    
public:
    Passenger(int from, int to);
    
    int getSourceFloor() const;
    int getTargetFloor() const;
    double getWaitTime() const;
    void updateWaitTime(double deltaTime);
    bool hasTimeout() const;
}; 