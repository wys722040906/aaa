#include "data_recorder.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

DataRecorder::DataRecorder() : isLogging(false) {}

DataRecorder::~DataRecorder() {
    if (isLogging) {
        stopLogging();
    }
}

void DataRecorder::startLogging(const std::string& filename) {
    logFile.open(filename);
    if (logFile.is_open()) {
        isLogging = true;
        // 写入CSV头
        logFile << "时间戳,电梯ID,楼层,乘客数,状态,等待时间,能耗\n";
    }
}

void DataRecorder::stopLogging() {
    if (isLogging) {
        logFile.close();
        isLogging = false;
    }
}

void DataRecorder::recordState(const std::vector<Elevator>& elevators, double currentTime) {
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        ElevatorRecord record(
            currentTime,
            i,
            elevator.getCurrentFloor(),
            elevator.getCurrentLoad(),
            elevator.getState(),
            0.0,  // 等待时间需要从乘客获取
            0.0   // 能耗需要从能源管理器获取
        );
        
        records.push_back(record);
        
        if (isLogging) {
            logFile << record.timestamp << ","
                   << record.elevatorId << ","
                   << record.floor << ","
                   << record.passengers << ","
                   << static_cast<int>(record.state) << ","
                   << record.waitTime << ","
                   << record.energyUsage << "\n";
        }
    }
}

DataRecorder::AnalysisResult DataRecorder::analyzeData() const {
    AnalysisResult result;
    if (records.empty()) return result;
    
    // 计算平均等待时间
    double totalWaitTime = 0;
    for (const auto& record : records) {
        totalWaitTime += record.waitTime;
        result.floorFrequency[record.floor - 1]++;
        result.totalPassengers += record.passengers;
        result.totalEnergyUsage += record.energyUsage;
    }
    result.averageWaitTime = totalWaitTime / records.size();
    
    // 找出高峰时段
    auto maxLoad = std::max_element(records.begin(), records.end(),
        [](const ElevatorRecord& a, const ElevatorRecord& b) {
            return a.passengers < b.passengers;
        });
    if (maxLoad != records.end()) {
        result.peakLoadTime = maxLoad->timestamp;
    }
    
    return result;
}

std::string DataRecorder::generateReport() const {
    auto result = analyzeData();
    std::stringstream ss;
    
    ss << "\n=== 运行数据分析报告 ===" << std::endl;
    ss << std::fixed << std::setprecision(2);
    ss << "总运行时间: " << records.back().timestamp - records.front().timestamp 
       << " 秒" << std::endl;
    ss << "平均等待时间: " << result.averageWaitTime << " 秒" << std::endl;
    ss << "高峰时刻: " << result.peakLoadTime << " 秒" << std::endl;
    ss << "总服务乘客数: " << result.totalPassengers << std::endl;
    ss << "总能耗: " << result.totalEnergyUsage << " kWh" << std::endl;
    
    ss << "\n楼层使用频率:" << std::endl;
    for (size_t i = 0; i < result.floorFrequency.size(); ++i) {
        ss << (i + 1) << "楼: " << std::string(result.floorFrequency[i] / 10, '*') 
           << " (" << result.floorFrequency[i] << ")" << std::endl;
    }
    
    return ss.str();
}

void DataRecorder::exportToCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    file << "时间戳,电梯ID,楼层,乘客数,状态,等待时间,能耗\n";
    for (const auto& record : records) {
        file << record.timestamp << ","
             << record.elevatorId << ","
             << record.floor << ","
             << record.passengers << ","
             << static_cast<int>(record.state) << ","
             << record.waitTime << ","
             << record.energyUsage << "\n";
    }
    
    file.close();
}

void DataRecorder::clear() {
    records.clear();
}

void DataRecorder::generateLoadGraph() const {
    if (records.empty()) return;
    
    std::cout << "\n=== 电梯负载图表 ===" << std::endl;
    
    // 按时间段统计平均负载
    const int TIME_SLOTS = 24; // 24个时间段
    std::vector<double> averageLoads(TIME_SLOTS, 0.0);
    std::vector<int> slotCounts(TIME_SLOTS, 0);
    
    double totalTime = records.back().timestamp - records.front().timestamp;
    double slotDuration = totalTime / TIME_SLOTS;
    
    for (const auto& record : records) {
        int slot = static_cast<int>((record.timestamp - records.front().timestamp) / slotDuration);
        if (slot >= 0 && slot < TIME_SLOTS) {
            averageLoads[slot] += record.passengers;
            slotCounts[slot]++;
        }
    }
    
    // 计算平均值并显示图表
    const int MAX_WIDTH = 50;
    double maxLoad = 0;
    
    for (int i = 0; i < TIME_SLOTS; ++i) {
        if (slotCounts[i] > 0) {
            averageLoads[i] /= slotCounts[i];
            maxLoad = std::max(maxLoad, averageLoads[i]);
        }
    }
    
    for (int i = 0; i < TIME_SLOTS; ++i) {
        int hour = (i * 24) / TIME_SLOTS;
        std::cout << std::setw(2) << hour << ":00 |";
        
        int barLength = static_cast<int>((averageLoads[i] * MAX_WIDTH) / maxLoad);
        std::cout << std::string(barLength, '#') 
                 << " " << std::fixed << std::setprecision(1) 
                 << averageLoads[i] << std::endl;
    }
}

void DataRecorder::generateWaitTimeGraph() const {
    if (records.empty()) return;
    
    std::cout << "\n=== 等待时间分布图 ===" << std::endl;
    
    // 统计等待时间分布
    const int TIME_RANGES = 6;  // 0-30s, 31-60s, 61-90s, 91-120s, 121-150s, >150s
    std::vector<int> waitTimeDistribution(TIME_RANGES, 0);
    
    for (const auto& record : records) {
        int range = static_cast<int>(record.waitTime / 30);
        if (range >= TIME_RANGES) range = TIME_RANGES - 1;
        waitTimeDistribution[range]++;
    }
    
    // 显示分布图
    const std::vector<std::string> labels = {
        "0-30s ", "31-60s", "61-90s", "91-120s", "121-150s", ">150s"
    };
    
    int maxCount = *std::max_element(waitTimeDistribution.begin(), waitTimeDistribution.end());
    const int MAX_WIDTH = 40;
    
    for (size_t i = 0; i < waitTimeDistribution.size(); ++i) {
        std::cout << labels[i] << " |";
        int barLength = static_cast<int>((waitTimeDistribution[i] * MAX_WIDTH) / maxCount);
        std::cout << std::string(barLength, '#') 
                 << " " << waitTimeDistribution[i] << std::endl;
    }
}

void DataRecorder::generateEnergyGraph() const {
    if (records.empty()) return;
    
    std::cout << "\n=== 能耗趋势图 ===" << std::endl;
    
    // 按时间段统计能耗
    const int TIME_SLOTS = 24;
    std::vector<double> energyUsage(TIME_SLOTS, 0.0);
    
    double totalTime = records.back().timestamp - records.front().timestamp;
    double slotDuration = totalTime / TIME_SLOTS;
    
    for (const auto& record : records) {
        int slot = static_cast<int>((record.timestamp - records.front().timestamp) / slotDuration);
        if (slot >= 0 && slot < TIME_SLOTS) {
            energyUsage[slot] += record.energyUsage;
        }
    }
    
    // 显示能耗趋势
    const int MAX_WIDTH = 50;
    double maxEnergy = *std::max_element(energyUsage.begin(), energyUsage.end());
    
    for (int i = 0; i < TIME_SLOTS; ++i) {
        int hour = (i * 24) / TIME_SLOTS;
        std::cout << std::setw(2) << hour << ":00 |";
        
        int barLength = static_cast<int>((energyUsage[i] * MAX_WIDTH) / maxEnergy);
        std::cout << std::string(barLength, '#') 
                 << " " << std::fixed << std::setprecision(2) 
                 << energyUsage[i] << " kWh" << std::endl;
    }
} 