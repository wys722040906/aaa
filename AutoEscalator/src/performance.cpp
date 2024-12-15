#include "performance.h"
#include <sstream>
#include <iomanip>

void Performance::startMeasure(const std::string& operation) {
    startTimes[operation] = Clock::now();
}

void Performance::endMeasure(const std::string& operation) {
    auto endTime = Clock::now();
    auto startTime = startTimes[operation];
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime).count() / 1000000.0; // 转换为秒
    
    auto& metric = metrics[operation];
    metric.totalTime += duration;
    metric.callCount++;
    metric.maxTime = std::max(metric.maxTime, duration);
    metric.minTime = std::min(metric.minTime, duration);
}

std::string Performance::getReport() const {
    std::stringstream ss;
    ss << "\n=== 性能统计报告 ===" << std::endl;
    ss << std::setw(20) << "操作" 
       << std::setw(15) << "调用次数"
       << std::setw(15) << "总时间(s)"
       << std::setw(15) << "平均时间(s)"
       << std::setw(15) << "最大时间(s)"
       << std::setw(15) << "最小时间(s)" << std::endl;
    ss << std::string(95, '-') << std::endl;
    
    for (const auto& [operation, metric] : metrics) {
        double avgTime = metric.callCount > 0 ? 
            metric.totalTime / metric.callCount : 0;
            
        ss << std::fixed << std::setprecision(6)
           << std::setw(20) << operation
           << std::setw(15) << metric.callCount
           << std::setw(15) << metric.totalTime
           << std::setw(15) << avgTime
           << std::setw(15) << metric.maxTime
           << std::setw(15) << (metric.callCount > 0 ? metric.minTime : 0)
           << std::endl;
    }
    
    return ss.str();
}

void Performance::reset() {
    metrics.clear();
    startTimes.clear();
} 