#include "settingsdialog.h"
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_maxPassengersSpinBox(new QSpinBox(this))
    , m_floorTravelTimeSpinBox(new QSpinBox(this))
    , m_idleTimeSpinBox(new QSpinBox(this))
    , m_dayDurationSpinBox(new QSpinBox(this))
    , m_requestIntervalSpinBox(new QSpinBox(this))
    , m_okButton(new QPushButton("确定", this))
    , m_cancelButton(new QPushButton("取消", this))
    , m_errorLabel(new QLabel(this))
{
    setupUI();
    setupConnections();
    setWindowTitle("系统设置");
}

void SettingsDialog::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 创建设置组
    createSettingsGroup();

    // 错误提示标签
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_errorLabel);

    // 按钮布局
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    // 设置窗口属性
    setModal(true);
    setFixedSize(400, 400);
}

void SettingsDialog::createSettingsGroup() {
    auto groupBox = new QGroupBox("电梯系统参数设置", this);
    auto layout = new QVBoxLayout(groupBox);

    // 设置各个SpinBox的范围和步长
    m_maxPassengersSpinBox->setRange(1, 50);
    m_maxPassengersSpinBox->setSuffix(" 人");
    m_maxPassengersSpinBox->setToolTip("设置电梯最大载客量");

    m_floorTravelTimeSpinBox->setRange(1, 10);
    m_floorTravelTimeSpinBox->setSuffix(" 秒/层");
    m_floorTravelTimeSpinBox->setToolTip("设置电梯上下楼每层所需时间");

    m_idleTimeSpinBox->setRange(5, 30);
    m_idleTimeSpinBox->setSuffix(" 秒");
    m_idleTimeSpinBox->setToolTip("设置电梯空闲时在某一层的最大停留时间");
    m_idleTimeSpinBox->setValue(10);

    m_dayDurationSpinBox->setRange(24, 2400);
    m_dayDurationSpinBox->setSingleStep(24);
    m_dayDurationSpinBox->setSuffix(" 秒/天");
    m_dayDurationSpinBox->setToolTip("设置模拟一天的总时长");
    m_dayDurationSpinBox->setValue(240);

    m_requestIntervalSpinBox->setRange(1, 60);
    m_requestIntervalSpinBox->setSuffix(" 秒");
    m_requestIntervalSpinBox->setToolTip("设置随机生成乘客请求的时间间隔");

    // 创建表单布局
    auto formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->addRow("电梯最大搭乘人数:", m_maxPassengersSpinBox);
    formLayout->addRow("上下楼时间:", m_floorTravelTimeSpinBox);
    formLayout->addRow("空闲停靠时间:", m_idleTimeSpinBox);
    formLayout->addRow("一天时间:", m_dayDurationSpinBox);
    formLayout->addRow("随机乘客请求间隔:", m_requestIntervalSpinBox);

    layout->addLayout(formLayout);
    ((QVBoxLayout*)this->layout())->addWidget(groupBox);
}

void SettingsDialog::setupConnections() {
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOKClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_dayDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::onDayDurationChanged);

    // 连接所有SpinBox的valueChanged信号到验证函数
    connect(m_maxPassengersSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::validateInput);
    connect(m_floorTravelTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::validateInput);
    connect(m_idleTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::validateInput);
    connect(m_dayDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::validateInput);
    connect(m_requestIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::validateInput);
}

void SettingsDialog::onOKClicked() {
    // 发送设置变更信号
    emit settingsChanged(
        m_maxPassengersSpinBox->value(),
        m_floorTravelTimeSpinBox->value(),
        m_idleTimeSpinBox->value(),
        m_dayDurationSpinBox->value(),
        m_requestIntervalSpinBox->value()
    );
    
    accept();
}

void SettingsDialog::onCancelClicked() {
    // 恢复原始值
    m_maxPassengersSpinBox->setValue(m_originalMaxPassengers);
    m_floorTravelTimeSpinBox->setValue(m_originalFloorTravelTime);
    m_idleTimeSpinBox->setValue(m_originalIdleTime);
    m_dayDurationSpinBox->setValue(m_originalDayDuration);
    m_requestIntervalSpinBox->setValue(m_originalRequestInterval);
    
    reject();
}

void SettingsDialog::onDayDurationChanged(int value) {
    if (value % 24 != 0) {
        m_errorLabel->setText("一天时间必须是24的倍数");
        m_okButton->setEnabled(false);
    } else {
        validateInput();
    }
}

void SettingsDialog::validateInput() {
    bool isValid = true;
    QString errorMsg;

    // 验证各项设置的合理性
    if (m_requestIntervalSpinBox->value() >= m_dayDurationSpinBox->value()) {
        isValid = false;
        errorMsg = "随机请求间隔不能大于等于一天时间！";
    }

    if (m_idleTimeSpinBox->value() >= m_dayDurationSpinBox->value()) {
        isValid = false;
        errorMsg = "空闲停靠时间不能大于等于一天时间！";
    }

    // 更新错误提示和确定按钮状态
    m_errorLabel->setText(errorMsg);
    m_okButton->setEnabled(isValid);
}

// Getter方法实现
int SettingsDialog::getMaxPassengers() const {
    return m_maxPassengersSpinBox->value();
}

int SettingsDialog::getFloorTravelTime() const {
    return m_floorTravelTimeSpinBox->value();
}

int SettingsDialog::getIdleTime() const {
    return m_idleTimeSpinBox->value();
}

int SettingsDialog::getDayDuration() const {
    return m_dayDurationSpinBox->value();
}

int SettingsDialog::getRequestInterval() const {
    return m_requestIntervalSpinBox->value();
}

// Setter方法实现
void SettingsDialog::setMaxPassengers(int value) {
    m_originalMaxPassengers = value;
    m_maxPassengersSpinBox->setValue(value);
}

void SettingsDialog::setFloorTravelTime(int value) {
    m_originalFloorTravelTime = value;
    m_floorTravelTimeSpinBox->setValue(value);
}

void SettingsDialog::setIdleTime(int value) {
    m_originalIdleTime = value;
    m_idleTimeSpinBox->setValue(value);
}

void SettingsDialog::setDayDuration(int value) {
    m_originalDayDuration = value;
    m_dayDurationSpinBox->setValue(value);
}

void SettingsDialog::setRequestInterval(int value) {
    m_originalRequestInterval = value;
    m_requestIntervalSpinBox->setValue(value);
} 