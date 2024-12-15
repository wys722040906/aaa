#include "ElevatorWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>

ElevatorWidget::ElevatorWidget(QWidget *parent)
    : QWidget(parent)
    , currentState(ElevatorState::IDLE)
    , currentFloor(1)
    , targetFloor(1)
    , passengerCount(0)
    , maxPassengers(10)
    , isOverloaded(false)
    , isMalfunction(false)
    , currentPosition(1)
    , targetPosition(1)
{
    setMinimumSize(200, 600);
    
    // 初始化动画定时器
    animationTimer = new QTimer(this);
    animationTimer->setInterval(16);  // 约60fps
    connect(animationTimer, &QTimer::timeout, this, &ElevatorWidget::onAnimationTimer);
    animationTimer->start();
}

void ElevatorWidget::updateStatus(ElevatorState state, int floor, int passengers) {
    currentState = state;
    targetFloor = floor;
    passengerCount = passengers;
    isOverloaded = (passengers > maxPassengers);
    
    // 更新目标位置
    targetPosition = floor;
    
    update();
}

void ElevatorWidget::onAnimationTimer() {
    if (qAbs(currentPosition - targetPosition) > 0.01) {
        // 平滑移动到目标位置
        if (currentPosition < targetPosition) {
            currentPosition += ANIMATION_SPEED;
        } else {
            currentPosition -= ANIMATION_SPEED;
        }
        update();
    }
}

QColor ElevatorWidget::getStatusColor() const {
    if (isMalfunction) return Qt::yellow;
    if (isOverloaded) return Qt::red;
    return Qt::green;
}

void ElevatorWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制电梯井道
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, width() - 1, height() - 1);
    
    // 绘制楼层标记
    drawFloorIndicator(painter);
    
    // 绘制电梯轿厢
    drawElevatorCar(painter);
    
    // 绘制状态信息
    drawStatusInfo(painter);
}

void ElevatorWidget::drawElevatorCar(QPainter& painter) {
    // 计算电梯轿厢位置
    int elevatorHeight = height() / 20;  // 电梯高度
    int yPos = height() - (currentPosition * elevatorHeight);
    
    // 绘制电梯轿厢
    QRect elevatorRect(10, yPos - elevatorHeight, width() - 20, elevatorHeight);
    
    // 创建渐变效果
    QLinearGradient gradient(elevatorRect.topLeft(), elevatorRect.topRight());
    QColor baseColor = getStatusColor();
    gradient.setColorAt(0, baseColor.lighter());
    gradient.setColorAt(1, baseColor);
    
    painter.setBrush(gradient);
    painter.drawRect(elevatorRect);
    
    // 绘制电梯门
    painter.setPen(Qt::black);
    int doorWidth = elevatorRect.width() / 2;
    painter.drawLine(elevatorRect.left() + doorWidth, elevatorRect.top(),
                    elevatorRect.left() + doorWidth, elevatorRect.bottom());
}

void ElevatorWidget::drawFloorIndicator(QPainter& painter) {
    int floorHeight = height() / 20;
    for (int i = 1; i <= 20; ++i) {
        int yPos = height() - (i * floorHeight);
        painter.drawText(5, yPos, QString::number(i));
    }
}

void ElevatorWidget::drawStatusInfo(QPainter& painter) {
    QString status;
    switch (currentState) {
        case ElevatorState::MOVING_UP: status = "↑"; break;
        case ElevatorState::MOVING_DOWN: status = "↓"; break;
        case ElevatorState::DOOR_OPENING: status = "<<"; break;
        case ElevatorState::DOOR_CLOSING: status = ">>"; break;
        case ElevatorState::MAINTENANCE: status = "维护"; break;
        case ElevatorState::EMERGENCY: status = "紧急"; break;
        default: status = "待机"; break;
    }
    
    QString info = QString("Floor: %1\nPassengers: %2/%3\nStatus: %4")
                      .arg(currentFloor)
                      .arg(passengerCount)
                      .arg(maxPassengers)
                      .arg(status);
                      
    painter.drawText(10, 20, info);
} 