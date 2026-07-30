#include <QApplication>
#include <QMap>
#include <QFileInfo>
#include <QObject>
#include <QDir>
#include <QStyleFactory>
#include <QPointer>
#include <QMenu>
#include <QIcon>
#include <QLockFile>
#include <QMessageBox>
#include <QStandardPaths>
#include "log.hpp"
#include "tray.h"
#include "mainwindow.h"
#include "configmanager.h"
#include "globalinputlistener.h"
#include "convertcodetostring.hpp"
#include "launcher.hpp"
#include "settingsdialog.h"

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
    QPointer<SettingsDialog> settingsDlg;

    QMap<QString, MainWindow *> windows;
    QMap<QString, MainWindow *> hotkeyToWindow;
    QPointer<MainWindow> firstWindow;

    auto createWindows = [&](const QVector<LibraryConfig> &libs) {
        for (const auto &lib : libs) {
            if (!lib.enabled || lib.path.isEmpty())
                continue;
            if (!windows.contains(lib.path)) {
                auto *window = new MainWindow(&config, lib);
                windows[lib.path] = window;
                QFileInfo dirInfo(lib.path);
                window->setWindowTitle("Stickers Manager - " + dirInfo.fileName());
                window->hide();
            }
        }
        rebuildHotkeyMapping(windows, hotkeyToWindow);
        TrayIcon::instance()->updateShowMenu(libs);
        if (firstWindow.isNull() && !windows.isEmpty())
            firstWindow = windows.first();
    };

    auto removeStaleWindows = [&](const QVector<LibraryConfig> &libs) {
        QStringList active;
        for (const auto &lib : libs)
            if (lib.enabled && !lib.path.isEmpty())
                active.append(lib.path);
        QStringList toRemove;
        for (auto it = windows.begin(); it != windows.end(); ++it)
            if (!active.contains(it.key()))
                toRemove.append(it.key());
        for (const auto &path : toRemove) {
            delete windows[path];
            windows.remove(path);
        }
    };

    auto fullReload = [&]() {
        config.loadSettings();
        auto libs = config.getLibraries();
        removeStaleWindows(libs);
        createWindows(libs);

        // rescan all existing windows
        for (auto window : windows)
            window->reloadLibrary();
    };

    auto showSettings = [&](bool toggle = false) {
        if (toggle && settingsDlg && settingsDlg->isVisible()) {
            settingsDlg->close();
            return;
        }
        if (!settingsDlg) {
            auto *dlg = new SettingsDialog(&config);
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            QObject::connect(dlg, &QDialog::accepted, fullReload);
            QObject::connect(dlg, &QDialog::finished, [&]() { settingsDlg = nullptr; });
            settingsDlg = dlg;
        }
        settingsDlg->show();
        settingsDlg->raise();
        settingsDlg->activateWindow();
    };

    // Show settings dialog on first launch if no libraries configured
    auto libs = config.getLibraries();
    bool hasLib = false;
    for (const auto &lib : libs)
        if (lib.enabled && !lib.path.isEmpty() && QDir(lib.path).exists()) {
            hasLib = true;
            break;
        }

    if (!hasLib) {
        SettingsDialog tmpDlg(&config, nullptr);
        if (tmpDlg.exec() == QDialog::Accepted) {
            config.loadSettings();
            libs = config.getLibraries();
        }
    }

    createWindows(libs);

    TrayIcon::instance()->updateShowMenu(libs);

    QObject::connect(TrayIcon::instance()->showSubMenu, &QMenu::triggered, [&](QAction *action) {
        QString libraryPath = action->data().toString();
        if (windows.contains(libraryPath)) {
            MainWindow *window = windows[libraryPath];
            if (window->isHidden())
                window->showWindow();
            else
                window->hide();
        }
    });

    QObject::connect(
        TrayIcon::instance(), &TrayIcon::activated,
        [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason != QSystemTrayIcon::DoubleClick) return;

            QString target = config.getDoubleClickTarget();
            if (target == "settings") {
                showSettings(true);
                return;
            }

            MainWindow *win = nullptr;
            if (target == "first-library" || target.isEmpty()) {
                win = windows.isEmpty() ? nullptr : windows.first();
            } else {
                for (auto it = windows.begin(); it != windows.end(); ++it) {
                    if (QFileInfo(it.key()).fileName() == target) {
                        win = it.value();
                        break;
                    }
                }
            }

            if (win) {
                if (win->isHidden())
                    win->showWindow();
                else
                    win->hide();
            }
        });

    GlobalInputListener *listener = new GlobalInputListener();

    if (!hotkeyToWindow.isEmpty()) {
        QObject::connect(listener, &GlobalInputListener::keyReleased, [&](int keyCode, ModifierKeys modifiers) {
            QString keyName = keyCodeToKeyString(keyCode);
            QString modifiersName = modifiersToString(modifiers);
            QString hotkey = modifiersName + "+" + keyName;
            if (!modifiers) hotkey = keyName;

            for (auto it = hotkeyToWindow.begin(); it != hotkeyToWindow.end(); ++it) {
                if (ShortcutCompare::compareShortcutKeys(hotkey, it.key())) {
                    MainWindow *window = it.value();
                    if (window->isHidden())
                        window->showWindow();
                    else
                        window->hide();
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

    QObject::connect(TrayIcon::instance()->action_rescan, &QAction::triggered, fullReload);

    QObject::connect(TrayIcon::instance()->action_settings, &QAction::triggered, [&]() {
        showSettings(false);
    });

    TrayIcon::instance()->show();
    return app.exec();
}
