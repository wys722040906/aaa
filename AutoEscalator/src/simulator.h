#pragma once
#include "building.h"
#include "statistics.h"
#include <string>
#include "visualizer.h"
#include "monitor.h"
#include "performance.h"
#include "monitor_display.h"
#include "help_system.h"
#include "animation_controller.h"

class Simulator {
private:
    Building building;
    Statistics stats;
    double currentTime;
    double totalTime;
    bool isRunning;
    Visualizer visualizer;
    Monitor monitor;
    Performance performance;
    MonitorDisplay monitorDisplay;
    HelpSystem helpSystem;
    AnimationController animator;
    
    void checkRushHour();
    bool isNearTime(double current, double target);
    void generateDownwardRequests();
    void displayStatus();
    void endSimulation();
    std::string formatTime(double seconds);
    std::string getStateString(ElevatorState state);
    void handleHelpCommand(const std::string& command);
    
public:
    Simulator();
    
    void start();
    void reset();
    void update(double deltaTime);
    void generateRandomRequests();
    void loadRequestsFromFile(const std::string& filename);
    
    // 状态查询
    bool isSimulationRunning() const;
    double getCurrentTime() const;
    
    void handleManualRequest();
    void showConfigMenu();
    void adjustConfig();
    void showDispatcherMenu();
    void displayDispatcherStatistics() const;
    void showPerformanceReport() const;
    void showEnergyReport() const;
    void showMaintenanceMenu();
    void displayMaintenanceStatus() const;
    void showDataAnalysisMenu() const;
    void showHelp(const std::string& topic = "") const;
}; 