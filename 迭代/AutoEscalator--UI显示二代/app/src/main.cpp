#include <QApplication>
#include <QFile>
#include <QIODevice>
#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 加载样式表
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
    
    // 创建登录窗口
    LoginWindow* loginWindow = new LoginWindow();
    MainWindow* mainWindow = new MainWindow();
    
    // 连接登录成功信号
    QObject::connect(loginWindow, &LoginWindow::loginSuccess, [=]() {
        mainWindow->show();
        loginWindow->close();
    });
    
    loginWindow->show();
    
    return app.exec();
}   