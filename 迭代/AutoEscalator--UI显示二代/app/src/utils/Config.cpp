#include "utils/Config.h"

Config::Config() : settings("ElevatorSystem", "Config") {
    loadSettings();
}

Config& Config::instance() {
    static Config instance;
    return instance;
}

void Config::setElevatorRange(int id, int minFloor, int maxFloor) {
    settings.beginGroup(QString("Elevator_%1").arg(id));
    settings.setValue("minFloor", minFloor);
    settings.setValue("maxFloor", maxFloor);
    settings.endGroup();
}

void Config::setElevatorSpeed(int id, double speed) {
    settings.beginGroup(QString("Elevator_%1").arg(id));
    settings.setValue("speed", speed);
    settings.endGroup();
}

void Config::setDoorTime(double time) {
    settings.setValue("doorTime", time);
}

int Config::getMinFloor(int id) const {
    settings.beginGroup(QString("Elevator_%1").arg(id));
    int value = settings.value("minFloor", 1).toInt();
    settings.endGroup();
    return value;
}

int Config::getMaxFloor(int id) const {
    settings.beginGroup(QString("Elevator_%1").arg(id));
    int value = settings.value("maxFloor", 20).toInt();
    settings.endGroup();
    return value;
}

double Config::getElevatorSpeed(int id) const {
    settings.beginGroup(QString("Elevator_%1").arg(id));
    double value = settings.value("speed", 1.0).toDouble();
    settings.endGroup();
    return value;
}

double Config::getDoorTime() const {
    return settings.value("doorTime", 0.5).toDouble();
}

void Config::saveSettings() {
    settings.sync();
}

void Config::loadSettings() {
    // 如果是首次运行，设置默认值
    if (settings.allKeys().isEmpty()) {
        // 设置默认电梯范围
        setElevatorRange(0, 1, 20);  // 第一个电梯覆盖所有楼层
        setElevatorRange(1, 1, 10);  // 第二个电梯低层
        setElevatorRange(2, 5, 15);  // 第三个电梯中层
        setElevatorRange(3, 10, 20); // 第四个电梯高层
        
        // 设置默认速度
        for (int i = 0; i < 4; ++i) {
            setElevatorSpeed(i, 1.0);
        }
        
        // 设置默认开关门时间
        setDoorTime(0.5);
        
        saveSettings();
    }
} 