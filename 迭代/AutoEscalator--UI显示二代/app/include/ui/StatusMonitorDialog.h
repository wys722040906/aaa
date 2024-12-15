#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QTimer>
#include "core/ElevatorDispatcher.h"
#include "core/PassengerManager.h"

class StatusMonitorDialog : public QDialog {
    Q_OBJECT
public:
    StatusMonitorDialog(ElevatorDispatcher* dispatcher, 
                       PassengerManager* passengerManager,
                       QWidget* parent = nullptr);

private slots:
    void updateStatus();

private:
    void setupUi();
    
    QTableWidget* statusTable;
    QTimer* updateTimer;
    ElevatorDispatcher* dispatcher;
    PassengerManager* passengerManager;
}; 