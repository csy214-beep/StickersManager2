/*
 * PLauncher - Live2D Virtual Desktop Partner
 * https://gitee.com/Pfolg/plauncher
 * https://sourceforge.net/projects/pfolg-plauncher/
 * Copyright (c) 2025 SY Cheng
 *
 * GPL v3 License
 * https://gnu.ac.cn/licenses/gpl-3.0.html
 */
#include "tray.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QTime>
#include <QDebug>
#include <QFile>
#include "launcher.hpp"
#include "custommenu.h"

// 初始化静态成员变量
TrayIcon *TrayIcon::m_instance = nullptr;

TrayIcon *TrayIcon::instance()
{
    if (!m_instance)
    {
        m_instance = new TrayIcon();
    }
    return m_instance;
}

void TrayIcon::showMessage(const QString &title, const QString &msg,
                           MessageIcon icon, int timeout)
{
    if (m_instance && !m_instance->m_silentMode)
    {
        m_instance->QSystemTrayIcon::showMessage(title, msg, icon, timeout);
    }
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    QString ico_path = ":/assets/st.png";
    QFileInfo ico(ico_path);
    if (ico.exists())
    {
        qDebug() << "ico found";
        setIcon(QIcon(ico_path));
    }
    else
    {
        qDebug() << "icon not found";
        QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
        setIcon(icon);
    }

    setToolTip("Stickers Manager " + Version);

    // 创建右键菜单
    menu = new CustomMenu();

    showSubMenu = new CustomMenu("Show");
    action_settings = new QAction("Settings", this);
    action_openRepo = new QAction("Pictures", this);
    action_rescan = new QAction("Rescan", this);
    QAction *action_openPath = new QAction("Open App Dir", this);
    QAction *openGithubAction = new QAction("GitHub", this);
    QAction *exitAction = new QAction("Exit", this);

    // 连接信号和槽（保持原有连接不变）
    connect(exitAction, &QAction::triggered, []()
            { QCoreApplication::quit(); });

    connect(action_openPath, &QAction::triggered, []()
            {
        QString appDir = QCoreApplication::applicationDirPath();
        launch(appDir); });
    connect(openGithubAction, &QAction::triggered, []()
            {
        QString githubUrl = "https://github.com/csy214-beep/StickersManager2";
        launch(githubUrl); });

    menu->addMenu(showSubMenu);
    menu->addActions({action_rescan, action_settings, action_openPath, action_openRepo});
    menu->addSeparator();
    menu->addAction(openGithubAction);
    menu->addSeparator();
    menu->addAction(exitAction);

    // 设置托盘图标的菜单
    this->setContextMenu(menu);

    // 显示托盘图标
    this->show();

    qDebug() << "TrayIcon singleton initialized";
}

void TrayIcon::updateShowMenu(const QVector<LibraryConfig> &libraries)
{
    showSubMenu->clear();

    for (int i = 0; i < libraries.size(); ++i)
    {
        const LibraryConfig &lib = libraries[i];
        if (!lib.enabled)
            continue;

        QFileInfo dirInfo(lib.path);
        QString menuText = dirInfo.fileName();
        if (menuText.isEmpty())
        {
            menuText = lib.path;
        }

        QAction *action = new QAction(menuText, this);
        action->setData(lib.path);
        showSubMenu->addAction(action);
    }
}

TrayIcon::~TrayIcon()
{
    delete menu;
    m_instance = nullptr;
}
