#pragma once
#include <QMainWindow>
#include <QVector>
#include <QTimer>
#include "ui/ElevatorWidget.h"
#include "core/ElevatorDispatcher.h"
#include "core/PassengerManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void showElevatorConfig();
    void handleEmergencyMode(bool enabled);
    void updateStatistics();
    void showFloorSettings();

private:
    void setupUi();
    void setupElevators();
    void createToolbar();
    void createStatusBar();
    void setupPassengerSimulation();

    QVector<ElevatorWidget*> elevatorWidgets;
    ElevatorDispatcher* dispatcher;
    QTimer* statsTimer;
    PassengerManager* passengerManager;
    QAction* peakHourAction;
}; 