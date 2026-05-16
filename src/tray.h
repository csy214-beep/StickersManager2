#ifndef TRAY_H
#define TRAY_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "configmanager.h"

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    QString Version = "ver20260517.8";

    TrayIcon(const TrayIcon &) = delete;
    TrayIcon &operator=(const TrayIcon &) = delete;

    static TrayIcon *instance();

    static void showMessage(const QString &title, const QString &msg,
                            MessageIcon icon = QSystemTrayIcon::Information,
                            int timeout = 10000);

    QAction *action_settings;
    QAction *action_openRepo;
    QAction *action_rescan;

    QMenu *showSubMenu;

    void updateShowMenu(const QVector<LibraryConfig> &libraries);

private:
    TrayIcon(QObject *parent = nullptr);
    ~TrayIcon();

    static TrayIcon *m_instance;

    QMenu *menu;
};

#endif // TRAY_H
