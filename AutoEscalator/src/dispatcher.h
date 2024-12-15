#pragma once
#include "elevator.h"
#include "passenger.h"
#include <vector>

class Dispatcher {
public:
    enum class Strategy {
        NEAREST_FIRST,    // 最近电梯优先
        LOAD_BALANCED,    // 负载均衡
        ENERGY_SAVING     // 节能模式
    };
    
private:
    Strategy currentStrategy;
    
    struct Statistics {
        int totalAssignments;
        int successfulAssignments;
        double averageWaitTime;
        double averageDistance;
    };
    
    Statistics stats;
    
public:
    Dispatcher(Strategy strategy = Strategy::NEAREST_FIRST);
    
    void setStrategy(Strategy strategy);
    Strategy getStrategy() const;
    
    // 为乘客分配最合适的电梯
    int assignElevator(const std::vector<Elevator>& elevators, 
                      const Passenger& passenger);
                      
    // 获取策略名称
    static std::string getStrategyName(Strategy strategy);
    
    void resetStatistics();
    const Statistics& getStatistics() const;
}; 