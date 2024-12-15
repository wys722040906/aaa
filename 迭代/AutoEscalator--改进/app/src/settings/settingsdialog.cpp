#include "settingsdialog.h"
#include <QMessageBox>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_maxPassengersSpinBox(new QSpinBox(this))
    , m_floorTravelTimeSpinBox(new QSpinBox(this))
    , m_idleTimeSpinBox(new QSpinBox(this))
    , m_dayDurationSpinBox(new QSpinBox(this))
    , m_requestIntervalSpinBox(new QSpinBox(this))
    , m_randomPassengerSpinBox(new QSpinBox(this))
    , m_peakPassengerSpinBox(new QSpinBox(this))
    , m_doorTimeSpinBox(new QSpinBox(this))
    , m_okButton(new QPushButton("确定", this))
    , m_cancelButton(new QPushButton("取消", this))
    , m_errorLabel(new QLabel(this))
    , m_originalMaxPassengers(12)
    , m_originalFloorTravelTime(5)
    , m_originalIdleTime(10)
    , m_originalDayDuration(240)
    , m_originalRequestInterval(10)
    , m_originalDoorTime(1000)
{
    // 确保所有指针都已正确初始化
    if (!m_maxPassengersSpinBox || !m_floorTravelTimeSpinBox || !m_idleTimeSpinBox ||
        !m_dayDurationSpinBox || !m_requestIntervalSpinBox || !m_randomPassengerSpinBox ||
        !m_peakPassengerSpinBox || !m_doorTimeSpinBox || !m_okButton || !m_cancelButton ||
        !m_errorLabel) {
        QMessageBox::critical(this, "错误", "初始化设置对话框组件失败");
        return;
    }

    setupUI();
    setupConnections();
    setWindowTitle("系统设置");
    
    // 设置初始值
    m_maxPassengersSpinBox->setValue(m_originalMaxPassengers);
    m_floorTravelTimeSpinBox->setValue(m_originalFloorTravelTime);
    m_idleTimeSpinBox->setValue(m_originalIdleTime);
    m_dayDurationSpinBox->setValue(m_originalDayDuration);
    m_requestIntervalSpinBox->setValue(m_originalRequestInterval);
    m_randomPassengerSpinBox->setValue(3);
    m_peakPassengerSpinBox->setValue(15);
    m_doorTimeSpinBox->setValue(m_originalDoorTime);
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
    auto layout = new QFormLayout(groupBox);
    layout->setSpacing(10);

    // 设置各个SpinBox的范围和步长
    m_maxPassengersSpinBox->setRange(1, 50);
    m_maxPassengersSpinBox->setSuffix(" 人");
    m_maxPassengersSpinBox->setToolTip("设置电梯最大载客量");

    m_floorTravelTimeSpinBox->setRange(1, 10);
    m_floorTravelTimeSpinBox->setSuffix(" 秒/层");
    m_floorTravelTimeSpinBox->setToolTip("设置电梯上下楼每层所需时间");

    m_doorTimeSpinBox->setRange(100, 2000);  // 0.1-2秒，以毫秒为单位
    m_doorTimeSpinBox->setSingleStep(100);
    m_doorTimeSpinBox->setSuffix(" ms");
    m_doorTimeSpinBox->setToolTip("设置电梯开关门时间");

    // 添加到表单布局（按照合理的顺序排列）
    layout->addRow("电梯最大搭乘人数:", m_maxPassengersSpinBox);
    layout->addRow("上下楼时间:", m_floorTravelTimeSpinBox);
    layout->addRow("开关门时间:", m_doorTimeSpinBox);
    layout->addRow("空闲停靠时间:", m_idleTimeSpinBox);
    layout->addRow("一天时间:", m_dayDurationSpinBox);
    layout->addRow("随机乘客请求间隔:", m_requestIntervalSpinBox);
    layout->addRow("随机乘客生成个数:", m_randomPassengerSpinBox);
    layout->addRow("高峰期乘客个数:", m_peakPassengerSpinBox);

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
    // 发送设置更信号
    emit settingsChanged(
        m_maxPassengersSpinBox->value(),
        m_floorTravelTimeSpinBox->value(),
        m_idleTimeSpinBox->value(),
        m_dayDurationSpinBox->value(),
        m_requestIntervalSpinBox->value(),
        m_randomPassengerSpinBox->value(),
        m_peakPassengerSpinBox->value(),
        m_doorTimeSpinBox->value()  // 添加开关门时间参数
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
    m_doorTimeSpinBox->setValue(m_originalDoorTime);
    
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
    
    // 验证随机乘客数量不能大于等于15（高峰期数量）
    if (m_randomPassengerSpinBox->value() >= m_peakPassengerSpinBox->value()) {
        isValid = false;
        errorMsg = "普通时段的乘客生成数不应大于等于高峰期！";
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

int SettingsDialog::getRandomPassengerCount() const {
    return m_randomPassengerSpinBox->value();
}

int SettingsDialog::getPeakPassengerCount() const {
    return m_peakPassengerSpinBox->value();
}

int SettingsDialog::getDoorTime() const {
    return m_doorTimeSpinBox->value();
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

void SettingsDialog::setRandomPassengerCount(int count) {
    m_randomPassengerSpinBox->setValue(count);
}

void SettingsDialog::setPeakPassengerCount(int count) {
    m_peakPassengerSpinBox->setValue(count);
}

void SettingsDialog::setDoorTime(int milliseconds) {
    m_originalDoorTime = milliseconds;
    m_doorTimeSpinBox->setValue(milliseconds);
} 