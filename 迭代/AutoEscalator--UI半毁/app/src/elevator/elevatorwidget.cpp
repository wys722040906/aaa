#include "elevatorwidget.h"
#include <QPainter>
#include <QDebug>

const QString ElevatorWidget::ACTIVE_PATH_STYLE = 
    "background-color: #808080;"; // 灰色表示电梯在该层
const QString ElevatorWidget::INACTIVE_PATH_STYLE = 
    "background-color: white;";   // 白色表示电梯不在该层

ElevatorWidget::ElevatorWidget(int id, QWidget *parent)
    : QWidget(parent)
    , m_id(id)
    , m_currentFloor(1)
    , m_direction(Direction::IDLE)
    , m_passengerCount(0)
    , m_isMoving(false)
{
    setupUI();
    setupConnections();
    updateDisplay();
}

void ElevatorWidget::setupUI() {
    setFixedWidth(60);
    
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(2, 2, 2, 2);

    // 楼层显示
    m_floorLabel = new QLabel(this);
    m_floorLabel->setAlignment(Qt::AlignCenter);
    m_floorLabel->setStyleSheet("font-size: 12px; font-weight: bold;");

    // 方向图标
    m_directionIcon = new QLabel(this);
    m_directionIcon->setFixedSize(20, 20);
    m_directionIcon->setAlignment(Qt::AlignCenter);

    // 路径指示器
    m_pathIndicator = new QWidget(this);
    m_pathIndicator->setStyleSheet(INACTIVE_PATH_STYLE);
    m_pathIndicator->setFixedHeight(PATH_HEIGHT);
    m_pathIndicator->setFixedWidth(width());
    m_pathIndicator->setAutoFillBackground(true);

    // 状态显示框（显示当前楼层和乘客数）
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setFixedHeight(25);
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 3px 5px;
            font-size: 12px;
            font-weight: bold;
        }
    )");

    // 布局
    auto infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(m_floorLabel);
    infoLayout->addWidget(m_directionIcon);

    mainLayout->addLayout(infoLayout);
    mainLayout->addWidget(m_pathIndicator, 1);  // 路径指示器占据剩余空间
    mainLayout->addWidget(m_statusLabel);

    // 设置动画
    m_pathAnimation = new QPropertyAnimation(m_pathIndicator, "geometry", this);
    m_pathAnimation->setDuration(TRANSITION_DURATION);

    // 设置过渡定时器
    m_transitionTimer = new QTimer(this);
    m_transitionTimer->setInterval(TRANSITION_DURATION);
    m_transitionTimer->setSingleShot(true);
}

void ElevatorWidget::setupConnections() {
    connect(m_pathAnimation, &QPropertyAnimation::finished,
            this, &ElevatorWidget::onFloorTransitionFinished);
    connect(m_transitionTimer, &QTimer::timeout,
            this, &ElevatorWidget::onFloorTransitionFinished);
}

void ElevatorWidget::setCurrentFloor(int floor) {
    if (floor < 1 || floor > 14 || floor == m_currentFloor) return;

    int oldFloor = m_currentFloor;
    m_currentFloor = floor;
    
    animateTransition(oldFloor, floor);
    updateDisplay();
    emit floorChanged(m_id, floor);
}

void ElevatorWidget::setDirection(Direction direction) {
    if (m_direction == direction) return;
    
    m_direction = direction;
    updateDirectionIcon();
    emit directionChanged(m_id, direction);
}

void ElevatorWidget::setPassengerCount(int count) {
    if (count < 0 || count == m_passengerCount) return;
    
    m_passengerCount = count;
    updateDisplay();
    emit passengerCountChanged(m_id, count);
}

void ElevatorWidget::setPathStatus(int floor, bool isActive) {
    if (floor < 1 || floor > 14) return;
    
    m_pathIndicator->setStyleSheet(
        isActive ? ACTIVE_PATH_STYLE : INACTIVE_PATH_STYLE
    );
    m_pathIndicator->update();
}

void ElevatorWidget::updateDirectionIcon() {
    switch (m_direction) {
        case Direction::UP:
            m_directionIcon->setStyleSheet(R"(
                QLabel {
                    background-color: transparent;
                    image: url(:/icons/up_arrow.png);
                }
            )");
            break;
        case Direction::DOWN:
            m_directionIcon->setStyleSheet(R"(
                QLabel {
                    background-color: transparent;
                    image: url(:/icons/down_arrow.png);
                }
            )");
            break;
        case Direction::IDLE:
            m_directionIcon->setStyleSheet(R"(
                QLabel {
                    background-color: transparent;
                    image: url(:/icons/circle.png);
                }
            )");
            break;
    }
}

void ElevatorWidget::updateDisplay() {
    m_floorLabel->setText(QString::number(m_currentFloor));
    updateDirectionIcon();
    updateStatusLabel();
}

void ElevatorWidget::animateTransition(int fromFloor, int toFloor) {
    if (m_isMoving) return;
    m_isMoving = true;

    // 计算起始和结束位置
    int startY = (14 - fromFloor) * PATH_HEIGHT;
    int endY = (14 - toFloor) * PATH_HEIGHT;

    // 设置动画
    m_pathAnimation->setStartValue(QRect(0, startY, width(), PATH_HEIGHT));
    m_pathAnimation->setEndValue(QRect(0, endY, width(), PATH_HEIGHT));
    
    // 开始动画和定时器
    m_pathAnimation->start();
    m_transitionTimer->start();
    
    // 更新路径显示
    setPathStatus(fromFloor, false);
    setPathStatus(toFloor, true);
}

void ElevatorWidget::onFloorTransitionFinished() {
    m_isMoving = false;
    updateDisplay();
}

void ElevatorWidget::updateStatusLabel() {
    // 格式：当前楼层 | 乘客数
    QString status = QString("%1 | %2")
        .arg(m_currentFloor)   // 左侧：当前楼层
        .arg(m_passengerCount);  // 右侧：乘客数量
    m_statusLabel->setText(status);
} 