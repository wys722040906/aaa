#pragma once
#include <vector>

class Statistics {
private:
    std::vector<int> floorUsage;
    
public:
    Statistics();
    
    void recordPassenger(int fromFloor, int toFloor);
    void reset();
    void displayChart() const;
    const std::vector<int>& getFloorUsage() const;
}; 