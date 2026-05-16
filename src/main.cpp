#include <QApplication>
#include <QMap>
#include <QFileInfo>
#include <QObject>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QStyleFactory>
#include "log.hpp"
#include "tray.h"
#include "mainwindow.h"
#include "configmanager.h"
#include "globalinputlistener.h"
#include "convertcodetostring.hpp"
#include "launcher.hpp"

#define DEBUG_MODE false

static void rebuildHotkeyMapping(const QMap<QString, MainWindow *> &windows, QMap<QString, MainWindow *> &hotkeyToWindow) {
    hotkeyToWindow.clear();
    for (auto it = windows.begin(); it != windows.end(); ++it) {
        LibraryConfig libConfig = it.value()->getLibraryConfig();
        if (!libConfig.hotkey.isEmpty()) {
            hotkeyToWindow[libConfig.hotkey] = it.value();
        }
    }
}

int main(int argc, char *argv[]) {
    initLogFile();
    setLogLevel(LogLevel::Warning);
    if (!DEBUG_MODE) qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    app.setApplicationName("Stickers Manager");
    QString userName = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).fileName();
    userName = !userName.isEmpty() ? userName : "unknown";
    QLockFile lockFile(QDir::temp().absoluteFilePath(QString("%1_%2.lock").arg(userName).arg(app.applicationName())));

    if (!lockFile.tryLock(100))
    {
        QMessageBox::warning(nullptr, "Warning", "There is already an instance running!");
        return 1;
    }
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/assets/st.png"));
    QStringList styles = QStyleFactory::keys();
    if (styles.contains("Fusion", Qt::CaseInsensitive))
        app.setStyle("Fusion");

    ConfigManager config;

    auto libraries = config.getLibraries();
    bool hasValidLibrary = false;
    for (const auto &lib: libraries) {
        if (lib.enabled && !lib.path.isEmpty() && QDir(lib.path).exists()) {
            hasValidLibrary = true;
            break;
        }
    }

    if (!hasValidLibrary) {
        QMessageBox::information(nullptr, "Welcome to Stickers Manager",
                                 "Welcome to Stickers Manager!\n\n"
                                 "Please select your sticker library folder.\n"
                                 "The library should contain multiple subfolders, each representing a category.");

        QString libraryPath = QFileDialog::getExistingDirectory(nullptr,
                                                                "Select Sticker Library Folder", QDir::homePath());

        if (!libraryPath.isEmpty()) {
            LibraryConfig newLib(libraryPath, "Ctrl+Shift+E", true);
            config.addLibrary(newLib);
            config.saveConfig();
            qDebug() << "First launch, added sticker library:" << libraryPath;
        } else {
            QMessageBox::warning(nullptr, "Warning",
                                 "No sticker library folder selected, program will exit.");
            exit(0);
        }
    }

    libraries = config.getLibraries();

    QMap<QString, MainWindow *> windows;

    for (const auto &lib: libraries) {
        if (!lib.enabled || lib.path.isEmpty())
            continue;

        MainWindow *window = new MainWindow(&config, lib);
        windows[lib.path] = window;

        QFileInfo dirInfo(lib.path);
        QString title = "Stickers Manager - " + dirInfo.fileName();
        window->setWindowTitle(title);

        window->hide();
    }

    TrayIcon::instance()->updateShowMenu(libraries);

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

    QPointer<MainWindow> firstWindow = windows.isEmpty() ? nullptr : windows.first();

    QObject::connect(
        TrayIcon::instance(), &TrayIcon::activated,
        [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::DoubleClick) {
                if (firstWindow.isNull())
                    return;
                if (firstWindow->isHidden())
                    firstWindow->showWindow();
                else
                    firstWindow->hide();
            }
        });

    GlobalInputListener *listener = new GlobalInputListener();

    QMap<QString, MainWindow *> hotkeyToWindow;
    rebuildHotkeyMapping(windows, hotkeyToWindow);

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

    QObject::connect(TrayIcon::instance()->action_rescan, &QAction::triggered, [&]() {
        config.reloadFromDisk();
        auto newLibs = config.getLibraries();

        // create windows for new libraries
        for (const auto &lib : newLibs) {
            if (!lib.enabled || lib.path.isEmpty())
                continue;
            if (!windows.contains(lib.path)) {
                MainWindow *window = new MainWindow(&config, lib);
                windows[lib.path] = window;
                QFileInfo dirInfo(lib.path);
                window->setWindowTitle("Stickers Manager - " + dirInfo.fileName());
                window->hide();
            }
        }

        rebuildHotkeyMapping(windows, hotkeyToWindow);
        TrayIcon::instance()->updateShowMenu(newLibs);

        if (firstWindow.isNull() && !windows.isEmpty())
            firstWindow = windows.first();

        // rescan all existing windows
        for (auto window : windows) {
            window->reloadLibrary();
        }
    });

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
