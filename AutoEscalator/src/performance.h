#pragma once
#include <chrono>
#include <string>
#include <map>
#include <vector>

class Performance {
private:
    struct Metric {
        double totalTime;
        int callCount;
        double maxTime;
        double minTime;
        
        Metric() : totalTime(0), callCount(0), maxTime(0), minTime(999999) {}
    };
    
    std::map<std::string, Metric> metrics;
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    std::map<std::string, TimePoint> startTimes;
    
public:
    // 开始测量某个操作的时间
    void startMeasure(const std::string& operation);
    
    // 结束测量并记录结果
    void endMeasure(const std::string& operation);
    
    // 获取性能报告
    std::string getReport() const;
    
    // 重置统计数据
    void reset();
}; 