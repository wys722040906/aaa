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

    static const int MAX_FLOOR = 14;  // 最高楼层限制
    static const int MIN_FLOOR = 1;   // 最低楼层限制
    static constexpr double DEFAULT_SPEED = 2.0;  // 默认速度（秒/楼层）

private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    mutable QSettings settings;
}; 