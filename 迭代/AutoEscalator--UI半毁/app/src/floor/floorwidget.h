#ifndef FLOORWIDGET_H
#define FLOORWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMap>
#include <QPropertyAnimation>
#include "../common/types.h"  // 使用公共定义的PassengerRequest

class FloorWidget : public QWidget {
    Q_OBJECT

public:
    explicit FloorWidget(int floorNumber, QWidget *parent = nullptr);
    ~FloorWidget() = default;

    void setStatus(bool hasWaitingPassengers);
    void addPassengerRequest(int targetFloor, int count);
    void clearPassengerRequests();
    const QMap<int, int>& getPassengerRequests() const { return m_passengerRequests; }
    int getFloorNumber() const { return m_floorNumber; }
    void updatePassengerRequest(int targetFloor, int countDelta);

signals:
    void requestAdded(int currentFloor, int targetFloor, int count);

private slots:
    void onStatusBallClicked();
    void onAddRequestClicked();
    void onRequestListClickedOutside();

private:
    void setupUI();
    void setupConnections();
    void updateStatusBall();
    void showRequestList(bool show);
    void showAddRequestDialog();
    bool validateRequestInput(const QString& input, int& targetFloor, int& count);
    void updateRequestList();

    // UI组件
    QLabel* m_floorNumberLabel;
    QPushButton* m_statusBall;
    QListWidget* m_requestList;
    QPushButton* m_addRequestButton;
    QWidget* m_requestListContainer;
    QPropertyAnimation* m_animation;

    // 数据成员
    int m_floorNumber;
    bool m_hasWaitingPassengers;
    QMap<int, int> m_passengerRequests;  // targetFloor -> count
    bool m_isRequestListVisible;

    // 样式相关
    static const QString IDLE_STYLE;
    static const QString WAITING_STYLE;
    static const int BALL_SIZE = 20;
    static const int ANIMATION_DURATION = 200;
};

#endif // FLOORWIDGET_H 