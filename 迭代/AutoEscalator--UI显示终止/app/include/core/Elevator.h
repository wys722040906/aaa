#pragma once
#include <QObject>
#include <QTimer>
#include <queue>
#include <vector>
#include <memory>

class Elevator : public QObject {
    Q_OBJECT
public:
    enum class State {
        IDLE,
        MOVING_UP,
        MOVING_DOWN,
        DOOR_OPENING,
        DOOR_CLOSING,
        MALFUNCTION,
        EMERGENCY
    };

    struct Config {
        int minFloor;
        int maxFloor;
        double doorTime{0.5};    // 开关门时间（秒）
        double moveTime{2.0};    // 电梯移动速度（秒/楼层）
        int capacity{15};        // 电梯容量（人）
    };

    explicit Elevator(QObject* parent = nullptr);
    
    // 基本操作
    void moveToFloor(int floor);
    void addDestination(int floor);
    void openDoor();
    void closeDoor();
    
    // 状态查询
    State getState() const { return state; }
    int getCurrentFloor() const { return currentFloor; }
    int getPassengerCount() const { return passengerCount; }
    bool isOverloaded() const { return passengerCount > config.capacity; }
    
    // 配置相关
    void setConfig(const Config& newConfig) { config = newConfig; }
    const Config& getConfig() const { return config; }

signals:
    void stateChanged(State newState);
    void floorChanged(int newFloor);
    void doorOpened();
    void doorClosed();
    void overloaded();
    void malfunctionOccurred();

public slots:
    void handleEmergency();
    void simulateMalfunction();
    void addPassenger();
    void removePassenger();

private:
    State state{State::IDLE};
    int currentFloor{1};
    int targetFloor{1};
    int passengerCount{0};
    Config config;
    std::queue<int> destinations;  // 目标楼层队列
    
    void updateState(State newState);
    bool isValidFloor(int floor) const;
    void processNextDestination();
    
    // 定��器用于模拟电梯运行时间
    QTimer* moveTimer;
    QTimer* doorTimer;
}; 