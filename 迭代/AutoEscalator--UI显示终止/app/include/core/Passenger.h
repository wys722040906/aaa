#pragma once
#include <chrono>

class Passenger {
public:
    enum class Type {
        NORMAL,     // 普通乘客
        ELDERLY,    // 老年人
        WORKER      // 上班族
    };

    Passenger(int from, int to, Type type = Type::NORMAL)
        : fromFloor(from), toFloor(to), type(type),
          timestamp(std::chrono::system_clock::now()) {}

    int getFromFloor() const { return fromFloor; }
    int getToFloor() const { return toFloor; }
    Type getType() const { return type; }
    const std::chrono::system_clock::time_point& getTimestamp() const { return timestamp; }

private:
    int fromFloor;
    int toFloor;
    Type type;
    std::chrono::system_clock::time_point timestamp;
}; 