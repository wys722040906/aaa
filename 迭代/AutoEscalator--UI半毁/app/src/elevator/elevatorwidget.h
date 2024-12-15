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
    void updateDirectionIcon();
    void onFloorTransitionFinished();

private:
    void setupUI();
    void setupConnections();
    void updateDisplay();
    void animateTransition(int fromFloor, int toFloor);
    QString getDirectionIconPath() const;
    
    // UI组件
    QLabel* m_floorLabel;        // 显示当前楼层
    QLabel* m_directionIcon;     // 方向图标
    QWidget* m_pathIndicator;    // 电梯路径指示器
    QLabel* m_statusLabel;       // 状态显示标签
    
    // 电梯状态
    int m_id;                    // 电梯编号
    int m_currentFloor;          // 当前楼层
    Direction m_direction;       // 运行方向
    int m_passengerCount;       // 载客数量
    bool m_isMoving;            // 是否在移动
    
    // 动画相关
    QPropertyAnimation* m_pathAnimation;
    QTimer* m_transitionTimer;
    
    // 样式相关
    static const QString ACTIVE_PATH_STYLE;
    static const QString INACTIVE_PATH_STYLE;
    static const int PATH_HEIGHT = 30;
    static const int TRANSITION_DURATION = 5000; // 5秒过渡时间
    
    // 添加新的私有方法
    void updateStatusLabel();  // 更新状态显示
    QMap<int, QWidget*> m_pathSegments;  // 存储每层的路径段
};

#endif // ELEVATORWIDGET_H 