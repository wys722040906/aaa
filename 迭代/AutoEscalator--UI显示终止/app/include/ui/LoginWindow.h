#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

class LoginWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccess();
    void elevatorConfigRequired();

private slots:
    void handleLogin();
    void showElevatorConfig();

private:
    void setupUi();
    bool validateCredentials(const QString& username, const QString& password);

    // UI components
    QWidget* centralWidget;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QLabel* statusLabel;
}; 