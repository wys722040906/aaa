#include "ui/LoginWindow.h"
#include <QMessageBox>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
}

void LoginWindow::setupUi() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto layout = new QGridLayout(centralWidget);
    
    // 创建登录表单
    layout->addWidget(new QLabel("用户名:", this), 0, 0);
    usernameEdit = new QLineEdit(this);
    layout->addWidget(usernameEdit, 0, 1);
    
    layout->addWidget(new QLabel("密码:", this), 1, 0);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit, 1, 1);
    
    loginButton = new QPushButton("登录", this);
    layout->addWidget(loginButton, 2, 0, 1, 2);
    
    statusLabel = new QLabel(this);
    layout->addWidget(statusLabel, 3, 0, 1, 2);
    
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    
    setWindowTitle("电梯管理系统登录");
    setFixedSize(300, 200);
}

void LoginWindow::handleLogin() {
    if (validateCredentials(usernameEdit->text(), passwordEdit->text())) {
        emit loginSuccess();
        emit elevatorConfigRequired();
        close();
    } else {
        statusLabel->setText("用户名或密码错误！");
        statusLabel->setStyleSheet("color: red");
    }
}

bool LoginWindow::validateCredentials(const QString& username, const QString& password) {
    // 简单的验证逻辑，实际应用中应该使用更安全的方式
    return username == "admin" && password == "admin";
}

void LoginWindow::showElevatorConfig() {
    // 这个函数可以为空，因为它的功能已经在MainWindow中实现
} 