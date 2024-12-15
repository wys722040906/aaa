#pragma once
#include <QString>
#include <QSettings>

class Config {
public:
    static Config& instance();
    
    void setElevatorRange(int id, int minFloor, int maxFloor);
    void setElevatorSpeed(int id, double speed);
    void setDoorTime(double time);
    
    int getMinFloor(int id) const;
    int getMaxFloor(int id) const;
    double getElevatorSpeed(int id) const;
    double getDoorTime() const;
    
    void saveSettings();
    void loadSettings();

private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    mutable QSettings settings;
}; 