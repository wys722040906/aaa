#pragma once

#include <QMainWindow>
#include <vector>

class ElevatorWidget;
class LoginDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    std::vector<ElevatorWidget*> elevatorWidgets;
    LoginDialog* loginDialog;
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupUI();
    void updateElevatorStatus();
}; 