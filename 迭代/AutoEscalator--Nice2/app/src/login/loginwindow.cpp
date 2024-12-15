#include "loginwindow.h"
#include "../mainwindow/mainwindow.h"
#include <QMessageBox>
#include <QApplication>

LoginWindow::LoginWindow(QWidget *parent) 
    : QWidget(parent)
    , m_usernameEdit(new QLineEdit(this))
    , m_passwordEdit(new QLineEdit(this))
    , m_loginButton(new QPushButton("登录", this))
    , m_exitButton(new QPushButton("退出", this))
    , m_statusLabel(new QLabel(this))
{
    setupUI();
    setupConnections();
    setWindowTitle("电梯管理系统 - 登录");
}

void LoginWindow::setupUI() {
    // 设置窗口大小和位置
    resize(300, 200);
    setFixedSize(300, 200);

    // 创建主布局
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 用户名输入区域
    auto usernameLayout = new QHBoxLayout();
    auto usernameLabel = new QLabel("用户名:", this);
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(m_usernameEdit);
    m_usernameEdit->setPlaceholderText("请输入用户名");

    // 密码输入区域
    auto passwordLayout = new QHBoxLayout();
    auto passwordLabel = new QLabel("密码:", this);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(m_passwordEdit);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");

    // 按钮区域
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_exitButton);

    // 状态标签
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: red;");

    // 添加所有组件到主布局
    mainLayout->addLayout(usernameLayout);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addStretch();

    // 设置样式
    setStyleSheet(R"(
        QLineEdit {
            padding: 5px;
            border: 1px solid #ccc;
            border-radius: 3px;
        }
        QPushButton {
            padding: 5px 15px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 3px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton#exitButton {
            background-color: #f44336;
        }
        QPushButton#exitButton:hover {
            background-color: #da190b;
        }
    )");

    m_exitButton->setObjectName("exitButton");
}

void LoginWindow::setupConnections() {
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &LoginWindow::onExitClicked);
    
    // 按下回车键时触发登录
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

bool LoginWindow::validateCredentials(const QString& username, const QString& password) {
    // 默认用户名和密码
    return (username == "admin" && password == "123456");
}

void LoginWindow::onLoginClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("用户名和密码不能为空！");
        return;
    }

    if (validateCredentials(username, password)) {
        // 登录成功，创建并显示主窗口
        auto mainWindow = new MainWindow();
        mainWindow->show();
        this->hide();
    } else {
        m_statusLabel->setText("用户名或密码错误！");
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    }
}

void LoginWindow::onExitClicked() {
    QApplication::quit();
} 