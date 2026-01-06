#include <QApplication>

#include "log.hpp"
#include "tray.h"
#include "mainwindow.h"
#include "configmanager.h"


int main(int argc, char *argv[]) {
    initLogFile();
    // 安装自定义消息处理器
    qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/assets/st.png"));

    // 设置插件路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString pluginPath = appDir + "/plugins/imageformats";

    if (QDir(pluginPath).exists()) {
        app.addLibraryPath(pluginPath);
        qDebug() << "图像格式插件路径已设置:" << pluginPath;
    } else {
        qWarning() << "图像格式插件路径不存在:" << pluginPath;
    }

    ConfigManager config;
    // 主窗口
    MainWindow window(&config);
    window.show();
    TrayIcon::instance()->show();
    return app.exec();
}
