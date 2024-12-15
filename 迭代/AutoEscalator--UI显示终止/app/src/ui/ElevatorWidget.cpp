#include "ui/ElevatorWidget.h"
#include <QPainter>

ElevatorWidget::ElevatorWidget(Elevator* elevator, QWidget* parent)
    : QWidget(parent), elevator(elevator) {
    setupUi();
    
    if (elevator) {
        connect(elevator, &Elevator::stateChanged, this, &ElevatorWidget::handleStateChange);
        connect(elevator, &Elevator::floorChanged, this, &ElevatorWidget::updateDisplay);
        connect(elevator, &Elevator::overloaded, this, &ElevatorWidget::handleOverload);
        connect(elevator, &Elevator::malfunctionOccurred, this, &ElevatorWidget::handleMalfunction);
    }
}

void ElevatorWidget::setupUi() {
    auto layout = new QVBoxLayout(this);
    
    floorLabel = new QLabel(this);
    stateLabel = new QLabel(this);
    passengerLabel = new QLabel(this);
    
    layout->addWidget(floorLabel);
    layout->addWidget(stateLabel);
    layout->addWidget(passengerLabel);
    
    setMinimumSize(150, 300);
    updateDisplay();
}

void ElevatorWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(rect(), currentColor);
    QWidget::paintEvent(event);
}

void ElevatorWidget::updateDisplay() {
    if (!elevator) {
        floorLabel->setText("当前楼层: --");
        passengerLabel->setText("乘客数: --");
        return;
    }
    
    floorLabel->setText(QString("当前楼层: %1").arg(elevator->getCurrentFloor()));
    passengerLabel->setText(QString("乘客数: %1").arg(elevator->getPassengerCount()));
    updateColor();
    update();
}

void ElevatorWidget::handleStateChange(Elevator::State state) {
    QString stateText;
    switch (state) {
        case Elevator::State::IDLE: stateText = "空闲"; break;
        case Elevator::State::MOVING_UP: stateText = "上行"; break;
        case Elevator::State::MOVING_DOWN: stateText = "下行"; break;
        case Elevator::State::DOOR_OPENING: stateText = "开门中"; break;
        case Elevator::State::DOOR_CLOSING: stateText = "关门中"; break;
        case Elevator::State::MALFUNCTION: stateText = "故障"; break;
        case Elevator::State::EMERGENCY: stateText = "紧急状态"; break;
    }
    stateLabel->setText(QString("状态: %1").arg(stateText));
    updateColor();
}

void ElevatorWidget::handleOverload() {
    currentColor = Qt::red;
    update();
}

void ElevatorWidget::handleMalfunction() {
    currentColor = Qt::yellow;
    update();
}

void ElevatorWidget::updateColor() {
    if (elevator->getState() == Elevator::State::MALFUNCTION) {
        currentColor = Qt::yellow;
    } else if (elevator->isOverloaded()) {
        currentColor = Qt::red;
    } else {
        currentColor = Qt::white;
    }
}

void ElevatorWidget::setElevator(Elevator* e) {
    elevator = e;
    if (elevator) {
        connect(elevator, &Elevator::stateChanged, this, &ElevatorWidget::handleStateChange);
        connect(elevator, &Elevator::floorChanged, this, &ElevatorWidget::updateDisplay);
        connect(elevator, &Elevator::overloaded, this, &ElevatorWidget::handleOverload);
        connect(elevator, &Elevator::malfunctionOccurred, this, &ElevatorWidget::handleMalfunction);
    }
    updateDisplay();
} 