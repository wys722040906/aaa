#pragma once
#include "building.h"
#include <vector>
#include <string>

class Monitor {
public:
    struct Alert {
        enum class Level {
            INFO,
            WARNING,
            ERROR
        };
        
        Level level;
        std::string message;
        double timestamp;
        
        Alert(Level l, const std::string& msg, double time)
            : level(l), message(msg), timestamp(time) {}
    };
    
private:
    std::vector<Alert> alerts;
    const Building& building;
    
    // 监控阈值
    static constexpr double MAX_WAIT_TIME_WARNING = 60.0;  // 60秒等待警告
    static constexpr double MAX_LOAD_RATIO = 0.8;         // 80%负载警告
    static constexpr int MAX_QUEUE_LENGTH = 10;           // 最大等待队列长度
    
public:
    explicit Monitor(const Building& b);
    
    void update(double currentTime);
    void addAlert(Alert::Level level, const std::string& message, double timestamp);
    void clearAlerts();
    
    void displayAlerts() const;
    const std::vector<Alert>& getAlerts() const;
    
private:
    void checkWaitingTimes(double currentTime);
    void checkElevatorLoads();
    void checkQueueLengths();
}; 