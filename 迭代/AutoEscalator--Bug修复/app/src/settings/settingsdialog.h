#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() = default;

    // 获取设置值
    int getMaxPassengers() const;
    int getFloorTravelTime() const;
    int getIdleTime() const;
    int getDayDuration() const;
    int getRequestInterval() const;
    int getRandomPassengerCount() const;
    int getPeakPassengerCount() const;
    int getDoorTime() const;

    // 设置初始值
    void setMaxPassengers(int value);
    void setFloorTravelTime(int value);
    void setIdleTime(int value);
    void setDayDuration(int value);
    void setRequestInterval(int value);
    void setRandomPassengerCount(int count);
    void setPeakPassengerCount(int count);
    void setDoorTime(int milliseconds);

signals:
    void settingsChanged(int maxPassengers, int floorTravelTime, 
                        int idleTime, int dayDuration, int requestInterval, 
                        int randomPassengerCount, int peakPassengerCount,
                        int doorTime);

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onDayDurationChanged(int value);
    void validateInput();

private:
    void setupUI();
    void setupConnections();
    void createSettingsGroup();

    // UI组件
    QSpinBox* m_maxPassengersSpinBox;
    QSpinBox* m_floorTravelTimeSpinBox;
    QSpinBox* m_idleTimeSpinBox;
    QSpinBox* m_dayDurationSpinBox;
    QSpinBox* m_requestIntervalSpinBox;
    QSpinBox* m_randomPassengerSpinBox;
    QSpinBox* m_peakPassengerSpinBox;
    QSpinBox* m_doorTimeSpinBox;
    
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QLabel* m_errorLabel;

    // 临时存储原始值，用于取消时恢复
    int m_originalMaxPassengers;
    int m_originalFloorTravelTime;
    int m_originalIdleTime;
    int m_originalDayDuration;
    int m_originalRequestInterval;
    int m_originalDoorTime;
};

#endif // SETTINGSDIALOG_H 