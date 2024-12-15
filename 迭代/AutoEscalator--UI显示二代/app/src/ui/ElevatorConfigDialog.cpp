#include "ui/ElevatorConfigDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>

ElevatorConfigDialog::ElevatorConfigDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
}

void ElevatorConfigDialog::setupUi() {
    setWindowTitle("电梯配置");
    
    auto layout = new QVBoxLayout(this);
    auto formLayout = new QFormLayout;
    
    // 为4个电梯创建配置控件
    for (int i = 0; i < 4; ++i) {
        auto minFloor = new QSpinBox(this);
        auto maxFloor = new QSpinBox(this);
        auto speed = new QSpinBox(this);
        
        minFloor->setRange(1, 100);
        maxFloor->setRange(1, 100);
        speed->setRange(1, 10);
        
        rangeInputs.append(qMakePair(minFloor, maxFloor));
        speedInputs.append(speed);
        
        formLayout->addRow(QString("电梯%1最低楼层").arg(i + 1), minFloor);
        formLayout->addRow(QString("电梯%1最高楼层").arg(i + 1), maxFloor);
        formLayout->addRow(QString("电梯%1速度").arg(i + 1), speed);
    }
    
    layout->addLayout(formLayout);
    
    auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    layout->addWidget(buttonBox);
}

QVector<ElevatorConfigDialog::ElevatorConfig> ElevatorConfigDialog::getConfigurations() const {
    QVector<ElevatorConfig> configs;
    for (int i = 0; i < rangeInputs.size(); ++i) {
        configs.append({
            rangeInputs[i].first->value(),
            rangeInputs[i].second->value(),
            static_cast<double>(speedInputs[i]->value())
        });
    }
    return configs;
} 