#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QVector>
#include <QPair>
#include <QDialogButtonBox>

class ElevatorConfigDialog : public QDialog {
    Q_OBJECT
public:
    struct ElevatorConfig {
        int minFloor;
        int maxFloor;
        double speed;
    };
    
    explicit ElevatorConfigDialog(QWidget* parent = nullptr);
    QVector<ElevatorConfig> getConfigurations() const;

private:
    void setupUi();
    QVector<QPair<QSpinBox*, QSpinBox*>> rangeInputs;
    QVector<QSpinBox*> speedInputs;
}; 