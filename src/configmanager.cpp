#include "configmanager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>

#include "launcher.hpp"
#include "tray.h"

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent) {
    // 确定配置文件路径
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir configDir = QDir(exeDir + "/.stickersmanager");
    QDir().mkpath(configDir.absolutePath());
    m_configPath = configDir.absolutePath() + "/config.json";

    if (!loadConfig()) {
        qWarning() << "使用默认配置";
    }
    connect(TrayIcon::instance()->action_settings, &QAction::triggered, [&]() {
        launch(m_configPath);
    });
    connect(TrayIcon::instance()->action_openRepo, &QAction::triggered, [&]() {
        QString p = m_config["libraryPath"].toString();
        launch(p);
    });
}

ConfigManager::~ConfigManager() {
    saveConfig();
}

bool ConfigManager::loadConfig() {
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        m_config = getDefaultConfig();
        return false;
    }

    QByteArray data = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        m_config = getDefaultConfig();
        return false;
    }

    m_config = doc.object();
    return true;
}

bool ConfigManager::saveConfig() {
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(m_config);
    configFile.write(doc.toJson());
    return true;
}

QJsonObject ConfigManager::getDefaultConfig() {
    QJsonObject config;

    // 基本设置
    config["libraryPath"] = "";
    config["hotkey"] = "Ctrl+Shift+E";

    // 窗口设置
    QJsonArray windowPos = {900, 50};
    QJsonArray windowSize = {600, 430};
    config["windowPosition"] = windowPos;
    config["windowSize"] = windowSize;

    // UI设置
    QJsonObject ui;
    ui["categoryButtonSize"] = 90;
    ui["gridCellSize"] = 120;
    ui["gridColumns"] = 3;
    config["ui"] = ui;
    // 行为设置
    QJsonObject behavior;
    behavior["copyOnDoubleClick"] = true;
    behavior["highlightOnClick"] = true;
    behavior["searchDelayMs"] = 300;
    config["behavior"] = behavior;

    // 性能设置
    QJsonObject performance;
    performance["thumbnailCacheSize"] = 200;
    performance["lazyLoadEnabled"] = true;
    config["performance"] = performance;

    return config;
}

// 配置项访问器实现
QString ConfigManager::getLibraryPath() const {
    return m_config["libraryPath"].toString();
}

void ConfigManager::setLibraryPath(const QString &path) {
    m_config["libraryPath"] = path;
    emit configChanged();
}

QString ConfigManager::getHotkey() const {
    return m_config["hotkey"].toString();
}

void ConfigManager::setHotkey(const QString &hotkey) {
    m_config["hotkey"] = hotkey;
    emit configChanged();
}

QSize ConfigManager::getWindowSize() const {
    QJsonArray sizeArray = m_config["windowSize"].toArray();
    if (sizeArray.size() == 2) {
        return QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
    }
    return QSize(600, 430);
}

void ConfigManager::setWindowSize(const QSize &size) {
    QJsonArray sizeArray;
    sizeArray.append(size.width());
    sizeArray.append(size.height());
    m_config["windowSize"] = sizeArray;
    emit configChanged();
}

QPoint ConfigManager::getWindowPosition() const {
    QJsonArray posArray = m_config["windowPosition"].toArray();
    if (posArray.size() == 2) {
        return QPoint(posArray[0].toInt(), posArray[1].toInt());
    }
    return QPoint(900, 50);
}

void ConfigManager::setWindowPosition(const QPoint &pos) {
    QJsonArray posArray;
    posArray.append(pos.x());
    posArray.append(pos.y());
    m_config["windowPosition"] = posArray;
    emit configChanged();
}

int ConfigManager::getCategoryButtonSize() const {
    QJsonObject ui = m_config["ui"].toObject();
    return ui["categoryButtonSize"].toInt(90);
}

int ConfigManager::getGridCellSize() const {
    QJsonObject ui = m_config["ui"].toObject();
    return ui["gridCellSize"].toInt(120);
}

int ConfigManager::getGridColumns() const {
    QJsonObject ui = m_config["ui"].toObject();
    return ui["gridColumns"].toInt(3);
}

bool ConfigManager::getCopyOnDoubleClick() const {
    QJsonObject behavior = m_config["behavior"].toObject();
    return behavior["copyOnDoubleClick"].toBool(true);
}

int ConfigManager::getThumbnailCacheSize() const {
    QJsonObject performance = m_config["performance"].toObject();
    return performance["thumbnailCacheSize"].toInt(200);
}
