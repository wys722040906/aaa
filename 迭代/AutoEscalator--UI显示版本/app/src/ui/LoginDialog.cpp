#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Login");
    
    auto layout = new QVBoxLayout(this);
    
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Username");
    layout->addWidget(usernameEdit);
    
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);
    
    loginButton = new QPushButton("Login", this);
    layout->addWidget(loginButton);
    
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
    // 简单的验证逻辑
    if (usernameEdit->text() == "admin" && passwordEdit->text() == "admin") {
        accept();
    } else {
        QMessageBox::warning(this, "Error", "Invalid username or password");
    }
} 