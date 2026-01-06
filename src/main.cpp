#include <QApplication>

#include "log.hpp"
#include "tray.h"
#include "mainwindow.h"
#include "configmanager.h"


int main(int argc, char *argv[]) {
    initLogFile();
    // 设置日志级别
    setLogLevel(LogLevel::Warning);
    // 安装自定义消息处理器
    qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/assets/st.png"));

    ConfigManager config;
    // 主窗口
    MainWindow window(&config);

    TrayIcon::instance()->show();
    return app.exec();
}
