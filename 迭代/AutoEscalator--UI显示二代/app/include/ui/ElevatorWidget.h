#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "core/Elevator.h"

class ElevatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit ElevatorWidget(Elevator* elevator = nullptr, QWidget* parent = nullptr);
    
    Elevator* getElevator() const { return elevator; }
    void setElevator(Elevator* e);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateDisplay();
    void handleStateChange(Elevator::State state);
    void handleOverload();
    void handleMalfunction();

private:
    void setupUi();
    void updateColor();

    Elevator* elevator;
    QLabel* floorLabel;
    QLabel* stateLabel;
    QLabel* passengerLabel;
    
    QColor currentColor{Qt::white};
}; 