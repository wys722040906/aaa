#include "core/Elevator.h"
#include <QTimer>
#include <QRandomGenerator>

Elevator::Elevator(QObject* parent) : QObject(parent) {
    moveTimer = new QTimer(this);
    doorTimer = new QTimer(this);
    
    connect(moveTimer, &QTimer::timeout, this, [this]() {
        if (currentFloor < targetFloor) {
            currentFloor++;
        } else if (currentFloor > targetFloor) {
            currentFloor--;
        }
        
        emit floorChanged(currentFloor);
        
        if (currentFloor == targetFloor) {
            moveTimer->stop();
            openDoor();
        }
    });
    
    connect(doorTimer, &QTimer::timeout, this, [this]() {
        if (state == State::DOOR_OPENING) {
            emit doorOpened();
            updateState(State::IDLE);
        } else if (state == State::DOOR_CLOSING) {
            emit doorClosed();
            processNextDestination();
        }
    });
}

void Elevator::moveToFloor(int floor) {
    if (!isValidFloor(floor) || state == State::MALFUNCTION) {
        return;
    }
    
    targetFloor = floor;
    if (currentFloor < targetFloor) {
        updateState(State::MOVING_UP);
    } else if (currentFloor > targetFloor) {
        updateState(State::MOVING_DOWN);
    }
    
    moveTimer->start(config.floorTime * 1000); // 转换为毫秒
}

void Elevator::addDestination(int floor) {
    if (isValidFloor(floor)) {
        destinations.push(floor);
        if (state == State::IDLE) {
            processNextDestination();
        }
    }
}

void Elevator::openDoor() {
    updateState(State::DOOR_OPENING);
    doorTimer->start(config.doorTime * 1000);
}

void Elevator::closeDoor() {
    updateState(State::DOOR_CLOSING);
    doorTimer->start(config.doorTime * 1000);
}

void Elevator::simulateMalfunction() {
    // 模拟1%的故障概率
    if (QRandomGenerator::global()->bounded(100) == 0) {
        updateState(State::MALFUNCTION);
        emit malfunctionOccurred();
    }
}

void Elevator::handleEmergency() {
    moveTimer->stop();
    doorTimer->stop();
    updateState(State::EMERGENCY);
    
    // 紧急情况下直接前往1楼
    if (currentFloor != 1) {
        moveToFloor(1);
    }
}

void Elevator::updateState(State newState) {
    if (state != newState) {
        state = newState;
        emit stateChanged(state);
    }
}

bool Elevator::isValidFloor(int floor) const {
    return floor >= config.minFloor && floor <= config.maxFloor;
}

void Elevator::processNextDestination() {
    if (!destinations.empty()) {
        int nextFloor = destinations.front();
        destinations.pop();
        moveToFloor(nextFloor);
    } else {
        updateState(State::IDLE);
    }
}

void Elevator::addPassenger() {
    passengerCount++;
    if (isOverloaded()) {
        emit overloaded();
    }
}

void Elevator::removePassenger() {
    if (passengerCount > 0) {
        passengerCount--;
    }
}