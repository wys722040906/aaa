#include "floorwidget.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QApplication>
#include <QScreen>

// 定义静态成员
const QString FloorWidget::IDLE_STYLE = 
    "QPushButton { background-color: #4CAF50; border-radius: 10px; }";
const QString FloorWidget::WAITING_STYLE = 
    "QPushButton { background-color: #f44336; border-radius: 10px; }";

FloorWidget::FloorWidget(int floorNumber, QWidget *parent)
    : QWidget(parent)
    , m_floorNumber(floorNumber)
    , m_hasWaitingPassengers(false)
    , m_isRequestListVisible(false)
{
    setupUI();
    setupConnections();
}

void FloorWidget::setupUI() {
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 2, 5, 2);

    // 楼层号标签使用固定大小
    m_floorNumberLabel = new QLabel(QString::number(m_floorNumber) + "层", this);
    m_floorNumberLabel->setFixedSize(40, 25);
    m_floorNumberLabel->setAlignment(Qt::AlignCenter);
    
    // 状态球按钮
    m_statusBall = new QPushButton(this);
    m_statusBall->setFixedSize(BALL_SIZE, BALL_SIZE);
    m_statusBall->setStyleSheet(IDLE_STYLE);
    
    // 请求列表容器
    m_requestListContainer = new QWidget(this);
    m_requestListContainer->setFixedHeight(0);
    
    auto containerLayout = new QVBoxLayout(m_requestListContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // 请求列表
    m_requestList = new QListWidget(m_requestListContainer);
    m_requestList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_requestList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_requestList->setFixedWidth(180);
    m_requestList->setVisible(false);
    m_requestList->setStyleSheet(R"(
        QListWidget {
            border: 1px solid #ccc;
            border-radius: 3px;
            background-color: white;
        }
        QListWidget::item {
            padding: 5px 10px;
            min-height: 20px;
        }
        QListWidget::item:first {
            background-color: #f5f5f5;
            font-weight: bold;
            padding-left: 10px;
        }
    )");
    
    // 设置列表控件的堆叠顺序
    m_requestList->setWindowFlags(m_requestList->windowFlags() | Qt::WindowStaysOnTopHint);
    m_requestList->raise();
    
    containerLayout->addWidget(m_requestList);
    
    // 添加请求按钮
    m_addRequestButton = new QPushButton("添加请求", m_requestListContainer);
    m_addRequestButton->setVisible(false);
    m_addRequestButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            padding: 5px;
            border-radius: 3px;
            min-height: 25px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
    )");
    containerLayout->addWidget(m_addRequestButton);
    
    // 添加到主布局
    layout->addWidget(m_floorNumberLabel);
    layout->addWidget(m_statusBall);
    layout->addWidget(m_requestListContainer, 1);
    
    // 重新添加动画初始化
    m_animation = new QPropertyAnimation(m_requestListContainer, "maximumHeight", this);
    m_animation->setDuration(ANIMATION_DURATION);
}

void FloorWidget::setupConnections() {
    connect(m_statusBall, &QPushButton::clicked, this, &FloorWidget::onStatusBallClicked);
    connect(m_addRequestButton, &QPushButton::clicked, this, &FloorWidget::onAddRequestClicked);
    
    // 点外部时隐藏请求列表
    qApp->installEventFilter(this);
}

void FloorWidget::setStatus(bool hasWaitingPassengers) {
    m_hasWaitingPassengers = hasWaitingPassengers;
    updateStatusBall();
}

void FloorWidget::updateStatusBall() {
    m_statusBall->setStyleSheet(m_hasWaitingPassengers ? WAITING_STYLE : IDLE_STYLE);
}

void FloorWidget::showRequestList(bool show) {
    if (show == m_isRequestListVisible) return;
    
    m_isRequestListVisible = show;
    m_requestList->setVisible(show);
    m_addRequestButton->setVisible(show);
    
    if (show) {
        m_requestListContainer->raise();
        m_requestList->raise();
    }
    
    // 设置动画目标高度
    int targetHeight = show ? 200 : 0;
    
    m_animation->setStartValue(m_requestListContainer->height());
    m_animation->setEndValue(targetHeight);
    m_animation->start();
}

void FloorWidget::onStatusBallClicked() {
    showRequestList(!m_isRequestListVisible);
    updateRequestList();
}

void FloorWidget::onRequestListClickedOutside() {
    showRequestList(false);
}

void FloorWidget::addPassengerRequest(int targetFloor, int count) {
    m_passengerRequests[targetFloor] += count;
    setStatus(!m_passengerRequests.isEmpty());
    updateRequestList();
}

void FloorWidget::clearPassengerRequests() {
    m_passengerRequests.clear();
    updateRequestList();
    setStatus(false);
    showRequestList(false);  // 确保列表收起
}

void FloorWidget::updateRequestList() {
    m_requestList->clear();
    
    // 添加标题行
    QListWidgetItem* titleItem = new QListWidgetItem("目标楼层  人数");
    titleItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_requestList->addItem(titleItem);
    
    // 添加请求数据
    QMapIterator<int, int> it(m_passengerRequests);
    while (it.hasNext()) {
        it.next();
        QString text = QString("%1楼      %2人")
                        .arg(it.key(), 2)
                        .arg(it.value(), 2);
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_requestList->addItem(item);
    }
}

void FloorWidget::onAddRequestClicked() {
    showAddRequestDialog();
}

void FloorWidget::showAddRequestDialog() {
    bool ok;
    QString input = QInputDialog::getText(this, "添加乘客请求",
        "请输入期望到达楼层及人数（格式：楼层 人数）：",
        QLineEdit::Normal, "", &ok);

    if (ok && !input.isEmpty()) {
        int targetFloor, count;
        if (validateRequestInput(input, targetFloor, count)) {
            addPassengerRequest(targetFloor, count);
            emit requestAdded(m_floorNumber, targetFloor, count);
        }
    }
}

bool FloorWidget::validateRequestInput(const QString& input, int& targetFloor, int& count) {
    QStringList parts = input.split(" ", Qt::SkipEmptyParts);
    if (parts.size() != 2) {
        QMessageBox::warning(this, "输入错误", "请按照格式输入：楼层 人数");
        return false;
    }

    bool ok1, ok2;
    targetFloor = parts[0].toInt(&ok1);
    count = parts[1].toInt(&ok2);

    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字");
        return false;
    }

    if (targetFloor < 1 || targetFloor > 14) {
        QMessageBox::warning(this, "输入错误", "楼层必须在1-14之间");
        return false;
    }

    if (targetFloor == m_floorNumber) {
        QMessageBox::warning(this, "输入错误", "目标楼层不能是当前楼层");
        return false;
    }

    if (count <= 0) {
        QMessageBox::warning(this, "输入错误", "人数必须大于0");
        return false;
    }

    return true;
}

void FloorWidget::updatePassengerRequest(int targetFloor, int countDelta) {
    // 更新请求数量
    m_passengerRequests[targetFloor] += countDelta;
    
    // 如果请求数量为0或负数，移除该请求
    if (m_passengerRequests[targetFloor] <= 0) {
        m_passengerRequests.remove(targetFloor);
    }
    
    // 更新状态和显示
    setStatus(!m_passengerRequests.isEmpty());
    updateRequestList();
    
    // 如果没有请求了，隐藏请求列表
    if (m_passengerRequests.isEmpty()) {
        showRequestList(false);
    }
} 