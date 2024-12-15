#ifndef ELEVATORWIDGET_H
#define ELEVATORWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>

// 电梯运行方向枚举
enum class Direction {
    UP,
    DOWN,
    IDLE
};

class ElevatorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ElevatorWidget(int id, QWidget *parent = nullptr);
    ~ElevatorWidget() = default;

    // 设置电梯状态
    void setCurrentFloor(int floor);
    void setDirection(Direction direction);
    void setPassengerCount(int count);
    void setPathStatus(int floor, bool isActive);
    
    // 获取电梯状态
    int getCurrentFloor() const { return m_currentFloor; }
    Direction getDirection() const { return m_direction; }
    int getPassengerCount() const { return m_passengerCount; }
    int getId() const { return m_id; }

signals:
    void floorChanged(int elevatorId, int floor);
    void directionChanged(int elevatorId, Direction direction);
    void passengerCountChanged(int elevatorId, int count);

private slots:
    void onFloorTransitionFinished();

private:
    void setupUI();
    void setupConnections();
    void updateDisplay();
    void updateStatusLabel();
    
    // UI组件
    QLabel* m_statusLabel;       // 状态显示标签
    QMap<int, QWidget*> m_pathSegments;  // 存储每层的路径段
    
    // 电梯状态
    int m_id;                    // 电梯编号
    int m_currentFloor;          // 当前楼层
    Direction m_direction;       // 运行方向
    int m_passengerCount;       // 载客数量
    
    // 样式相关
    static const QString ACTIVE_PATH_STYLE;
    static const QString INACTIVE_PATH_STYLE;
    static const int PATH_HEIGHT = 30;
};

#endif // ELEVATORWIDGET_H 