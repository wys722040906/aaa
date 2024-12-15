#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() = default;

private slots:
    void onLoginClicked();
    void onExitClicked();

private:
    void setupUI();
    void setupConnections();
    bool validateCredentials(const QString& username, const QString& password);

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginButton;
    QPushButton* m_exitButton;
    QLabel* m_statusLabel;
};

#endif // LOGINWINDOW_H 