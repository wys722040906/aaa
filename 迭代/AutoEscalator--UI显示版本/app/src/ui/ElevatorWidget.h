#pragma once

#include <QWidget>
#include <core/Types.h>
#include <QTimer>

class ElevatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit ElevatorWidget(QWidget *parent = nullptr);
    void updateStatus(ElevatorState state, int floor, int passengers);

private slots:
    void onAnimationTimer();

private:
    void paintEvent(QPaintEvent *event) override;
    void drawElevatorCar(QPainter& painter);
    void drawFloorIndicator(QPainter& painter);
    void drawStatusInfo(QPainter& painter);
    QColor getStatusColor() const;
    
private:
    ElevatorState currentState;
    int currentFloor;
    int targetFloor;
    int passengerCount;
    int maxPassengers;
    bool isOverloaded;
    bool isMalfunction;
    
    // 动画相关
    QTimer* animationTimer;
    qreal currentPosition;  // 电梯的当前动画位置
    qreal targetPosition;   // 电梯的目标位置
    static constexpr qreal ANIMATION_SPEED = 0.1;  // 动画速度
}; 