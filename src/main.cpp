#include <QApplication>
#include <QMap>
#include <QFileInfo>
#include <QObject>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include "log.hpp"
#include "tray.h"
#include "mainwindow.h"
#include "configmanager.h"
#include "checkSingleInstance.hpp"
#include "globalinputlistener.h"
#include "convertcodetostring.hpp"
#include "launcher.hpp"

#define DEBUG_MODE false

int main(int argc, char *argv[]) {
    initLogFile();
    setLogLevel(LogLevel::Warning);
    if (!DEBUG_MODE) qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/assets/st.png"));

    ConfigManager config;
    PortListener portls;
    if (!portls.checkSingleInstance(config.getPort())) {
        exit(0);
    }

    // 检查是否是首次启动（没有配置的仓库）
    auto libraries = config.getLibraries();
    bool hasValidLibrary = false;
    for (const auto &lib: libraries) {
        if (lib.enabled && !lib.path.isEmpty() && QDir(lib.path).exists()) {
            hasValidLibrary = true;
            break;
        }
    }

    if (!hasValidLibrary) {
        // 首次启动，引导用户选择表情库
        QMessageBox::information(nullptr, "欢迎使用 Stickers Manager",
                                 "欢迎使用 Stickers Manager！\n\n"
                                 "请选择您的表情库文件夹。\n"
                                 "表情库应该包含多个子文件夹，每个子文件夹代表一个分类。");

        QString libraryPath = QFileDialog::getExistingDirectory(nullptr,
                                                                "选择表情库文件夹", QDir::homePath());

        if (!libraryPath.isEmpty()) {
            // 添加新的表情库配置
            LibraryConfig newLib(libraryPath, "Ctrl+Shift+E", true);
            config.addLibrary(newLib);
            config.saveConfig();
            qDebug() << "首次启动，已添加表情库:" << libraryPath;
        } else {
            QMessageBox::warning(nullptr, "警告",
                                 "未选择表情库文件夹，程序将退出。");
            exit(0);
        }
    }

    // 重新加载配置的仓库
    libraries = config.getLibraries();

    // 管理多个MainWindow实例
    QMap<QString, MainWindow *> windows;

    // 创建每个仓库创建一个窗口
    for (const auto &lib: libraries) {
        if (!lib.enabled || lib.path.isEmpty())
            continue;

        MainWindow *window = new MainWindow(&config, lib);
        windows[lib.path] = window;

        // 设置窗口标题
        QFileInfo dirInfo(lib.path);
        QString title = "Stickers Manager - " + dirInfo.fileName();
        window->setWindowTitle(title);

        // 初始隐藏窗口
        window->hide();
    }

    // 更新托盘菜单的Show子菜单
    TrayIcon::instance()->updateShowMenu(libraries);

    // 连接托盘菜单Show子菜单的触发信号
    QObject::connect(TrayIcon::instance()->showSubMenu, &QMenu::triggered, [&](QAction *action) {
        QString libraryPath = action->data().toString();
        if (windows.contains(libraryPath)) {
            MainWindow *window = windows[libraryPath];
            if (window->isHidden()) {
                window->showWindow();
            } else {
                window->hide();
            }
        }
    });

    // 托盘图标的双击显示第一个仓库窗口
    // 使用 QPointer 自动追踪窗口生命周期
    QPointer<MainWindow> firstWindow = windows.first();

    QObject::connect(
        TrayIcon::instance(), &TrayIcon::activated,
        [&, firstWindow](QSystemTrayIcon::ActivationReason reason) {
            // 显式捕获 QPointer 副本
            if (reason == QSystemTrayIcon::DoubleClick) {
                if (!firstWindow) // 窗口已被删除
                    return;
                if (firstWindow->isHidden())
                    firstWindow->showWindow();
                else
                    firstWindow->hide();
            }
        });

    // 创建全局热键监听器
    GlobalInputListener *listener = new GlobalInputListener();

    // 构建热键到窗口的映射
    QMap<QString, MainWindow *> hotkeyToWindow;
    for (auto it = windows.begin(); it != windows.end(); ++it) {
        LibraryConfig libConfig = it.value()->getLibraryConfig();
        if (!libConfig.hotkey.isEmpty()) {
            hotkeyToWindow[libConfig.hotkey] = it.value();
        }
    }

    // 连接热键信号
    if (!hotkeyToWindow.isEmpty()) {
        QObject::connect(listener, &GlobalInputListener::keyReleased, [&](int keyCode, ModifierKeys modifiers) {
            QString keyName = keyCodeToKeyString(keyCode);
            QString modifiersName = modifiersToString(modifiers);
            QString hotkey = modifiersName + "+" + keyName;
            if (!modifiers) hotkey = keyName;

            for (auto it = hotkeyToWindow.begin(); it != hotkeyToWindow.end(); ++it) {
                if (ShortcutCompare::compareShortcutKeys(hotkey, it.key())) {
                    MainWindow *window = it.value();
                    if (window->isHidden()) {
                        window->showWindow();
                    } else {
                        window->hide();
                    }
                    break;
                }
            }
        });

        if (!listener->startListening()) {
            qCritical() << "Failed to start global input listening";
        } else {
            qDebug() << "Global input listener is running with" << hotkeyToWindow.size() << "hotkeys";
        }
    }

    // 连接Rescan按钮 - 重新扫描所有仓库
    QObject::connect(TrayIcon::instance()->action_rescan, &QAction::triggered, [&]() {
        for (auto window: windows) {
            window->reloadLibrary();
        }
    });

    // 连接托盘菜单的Settings和Pictures按钮
    QObject::connect(TrayIcon::instance()->action_settings, &QAction::triggered,
                     [&]() { launch(config.getConfigPath()); });
    QObject::connect(TrayIcon::instance()->action_openRepo, &QAction::triggered, [&]() {
        auto libs = config.getLibraries();
        if (!libs.isEmpty()) {
            launch(libs.first().path);
        }
    });

    TrayIcon::instance()->show();
    return app.exec();
}
