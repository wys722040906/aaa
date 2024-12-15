#include "passenger.h"

Passenger::Passenger(int from, int to) 
    : sourceFloor(from), targetFloor(to), waitTime(0.0), isTimeout(false) {}

int Passenger::getSourceFloor() const {
    return sourceFloor;
}

int Passenger::getTargetFloor() const {
    return targetFloor;
}

double Passenger::getWaitTime() const {
    return waitTime;
}

void Passenger::updateWaitTime(double deltaTime) {
    waitTime += deltaTime;
    if (waitTime >= ElevatorConfig::MAX_WAIT_TIME) {
        isTimeout = true;
    }
}

bool Passenger::hasTimeout() const {
    return isTimeout;
} 