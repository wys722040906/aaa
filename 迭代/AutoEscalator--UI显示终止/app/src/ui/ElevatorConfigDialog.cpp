#include "ui/ElevatorConfigDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include "utils/Config.h"

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
        auto speed = new QDoubleSpinBox(this);
        
        // 设置楼层范围
        minFloor->setRange(Config::MIN_FLOOR, Config::MAX_FLOOR);
        maxFloor->setRange(Config::MIN_FLOOR, Config::MAX_FLOOR);
        
        // 设置速度范围和默认值
        speed->setRange(0.5, 5.0);  // 0.5-5秒/楼层
        speed->setValue(Config::DEFAULT_SPEED);
        speed->setSingleStep(0.1);
        
        // 第一个电梯固定运行区间
        if (i == 0) {
            minFloor->setValue(Config::MIN_FLOOR);
            maxFloor->setValue(Config::MAX_FLOOR);
            minFloor->setEnabled(false);
            maxFloor->setEnabled(false);
        } else {
            minFloor->setValue(Config::MIN_FLOOR);
            maxFloor->setValue(Config::MAX_FLOOR);
        }
        
        rangeInputs.append(qMakePair(minFloor, maxFloor));
        speedInputs.append(speed);
        
        formLayout->addRow(QString("电梯%1最低楼层").arg(i + 1), minFloor);
        formLayout->addRow(QString("电梯%1最高楼层").arg(i + 1), maxFloor);
        formLayout->addRow(QString("电梯%1速度(秒/楼层)").arg(i + 1), speed);
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