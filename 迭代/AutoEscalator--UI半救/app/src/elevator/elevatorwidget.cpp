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
{
    setupUI();
    setupConnections();
    updateDisplay();
}

void ElevatorWidget::setupUI() {
    setFixedWidth(60);
    
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);  // 设置为0以消除路径段之间的间隔
    mainLayout->setContentsMargins(2, 2, 2, 2);

    // 创建顶部显示组件
    m_floorLabel = new QLabel(this);
    m_floorLabel->setAlignment(Qt::AlignCenter);
    m_floorLabel->setStyleSheet("font-size: 12px; font-weight: bold;");

    m_directionIcon = new QLabel(this);
    m_directionIcon->setFixedSize(20, 20);
    m_directionIcon->setAlignment(Qt::AlignCenter);

    // 顶部信息布局
    auto infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(m_floorLabel);
    infoLayout->addWidget(m_directionIcon);

    // 路径显示容器
    auto pathContainer = new QWidget(this);
    auto pathLayout = new QVBoxLayout(pathContainer);
    pathLayout->setSpacing(0);
    pathLayout->setContentsMargins(0, 0, 0, 0);

    // 创建14层的路径段
    m_pathSegments.clear();
    for (int i = 14; i >= 1; --i) {
        auto pathSegment = new QWidget(pathContainer);
        pathSegment->setFixedHeight(PATH_HEIGHT);
        pathSegment->setStyleSheet(INACTIVE_PATH_STYLE);
        pathSegment->setAutoFillBackground(true);
        pathLayout->addWidget(pathSegment);
        m_pathSegments[i] = pathSegment;
    }

    // 状态显示框
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

    mainLayout->addLayout(infoLayout);
    mainLayout->addWidget(pathContainer, 1);
    mainLayout->addWidget(m_statusLabel);
}

void ElevatorWidget::setupConnections() {
    // 不再需要动画相关的连接
}

void ElevatorWidget::setCurrentFloor(int floor) {
    if (floor < 1 || floor > 14 || floor == m_currentFloor) return;

    // 清除旧楼层的高亮
    setPathStatus(m_currentFloor, false);
    
    // 更新当前楼层
    m_currentFloor = floor;
    
    // 设置新楼层的高亮
    setPathStatus(floor, true);
    
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
    
    if (auto pathSegment = m_pathSegments.value(floor)) {
        pathSegment->setStyleSheet(
            isActive ? ACTIVE_PATH_STYLE : INACTIVE_PATH_STYLE
        );
        pathSegment->update();
    }
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

void ElevatorWidget::onFloorTransitionFinished() {
    updateDisplay();
}

void ElevatorWidget::updateStatusLabel() {
    // 格式：当前楼层 | 乘客数
    QString status = QString("%1 | %2")
        .arg(m_currentFloor)   // 左侧：当前楼层
        .arg(m_passengerCount);  // 右侧：乘客数量
    m_statusLabel->setText(status);
} 