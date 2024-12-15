#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "elevator.h"

class DataRecorder {
public:
    struct ElevatorRecord {
        double timestamp;
        int elevatorId;
        int floor;
        int passengers;
        ElevatorState state;
        double waitTime;
        double energyUsage;
        
        ElevatorRecord(double time, int id, int f, int p, ElevatorState s, 
                      double wait, double energy)
            : timestamp(time), elevatorId(id), floor(f), passengers(p),
              state(s), waitTime(wait), energyUsage(energy) {}
    };
    
private:
    std::vector<ElevatorRecord> records;
    std::ofstream logFile;
    bool isLogging;
    
    // 分析数据
    struct AnalysisResult {
        double averageWaitTime;
        double peakLoadTime;
        double totalEnergyUsage;
        int totalPassengers;
        std::vector<int> floorFrequency;
        
        AnalysisResult() : averageWaitTime(0), peakLoadTime(0),
                          totalEnergyUsage(0), totalPassengers(0),
                          floorFrequency(14, 0) {}
    };
    
public:
    DataRecorder();
    ~DataRecorder();
    
    // 记录操作
    void startLogging(const std::string& filename);
    void stopLogging();
    void recordState(const std::vector<Elevator>& elevators, double currentTime);
    
    // 数据分析
    AnalysisResult analyzeData() const;
    
    // 生成报告
    std::string generateReport() const;
    void exportToCSV(const std::string& filename) const;
    
    // 图表生成
    void generateLoadGraph() const;    // 生成负载图表
    void generateWaitTimeGraph() const; // 生成等待时间图表
    void generateEnergyGraph() const;   // 生成能耗图表
    
    // 清除数据
    void clear();
}; 