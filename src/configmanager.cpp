#include "configmanager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>

#include "launcher.hpp"
#include "tray.h"

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent) {
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir configDir = QDir(exeDir + "/.stickersmanager");
    QDir().mkpath(configDir.absolutePath());
    m_configPath = configDir.absolutePath() + "/config.json";

    if (!loadConfig()) {
        qWarning() << "使用默认配置";
    }

    migrateToMultiLibrary();
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::loadConfig() {
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        m_config = getDefaultConfig();
        return false;
    }

    QByteArray data = configFile.readAll();
    configFile.close(); // 显式关闭文件

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        m_config = getDefaultConfig();
        return false;
    }

    m_config = doc.object();
    return true;
}

bool ConfigManager::saveConfig() {
    // 原子写入：先写入临时文件，再重命名覆盖原文件
    QString tmpPath = m_configPath + ".tmp";
    QFile tmpFile(tmpPath);
    if (!tmpFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QJsonDocument doc(m_config);
    qint64 bytesWritten = tmpFile.write(doc.toJson());
    tmpFile.close(); // 显式关闭临时文件

    if (bytesWritten == -1)
    {
        QFile::remove(tmpPath);
        return false;
    }

    // 删除原有配置文件（Windows 下 rename 不能直接覆盖）
    if (QFile::exists(m_configPath))
    {
        if (!QFile::remove(m_configPath))
        {
            QFile::remove(tmpPath);
            return false;
        }
    }

    // 重命名临时文件为正式配置文件
    if (!QFile::rename(tmpPath, m_configPath))
    {
        QFile::remove(tmpPath);
        return false;
    }

    return true;
}

void ConfigManager::migrateToMultiLibrary() {
    if (!m_config.contains("libraries")) {
        QJsonArray libraries;
        QString oldPath = m_config["libraryPath"].toString();
        if (!oldPath.isEmpty()) {
            QJsonObject libObj;
            libObj["path"] = oldPath;
            QJsonObject shortcuts = m_config["shortcuts"].toObject();
            libObj["hotkey"] = shortcuts["hotkey"].toString("Ctrl+Shift+E");
            libObj["enabled"] = shortcuts["useHotkey"].toBool(true);
            libraries.append(libObj);
        }
        m_config["libraries"] = libraries;
        saveConfig(); // 迁移时保存一次（仅首次运行）
    }
}

QVector<LibraryConfig> ConfigManager::getLibraries() const {
    QVector<LibraryConfig> result;
    QJsonArray libArray = m_config["libraries"].toArray();
    for (const QJsonValue &val: libArray) {
        QJsonObject obj = val.toObject();
        LibraryConfig lib;
        lib.path = obj["path"].toString();
        lib.hotkey = obj["hotkey"].toString();
        lib.enabled = obj["enabled"].toBool(true);
        result.append(lib);
    }
    return result;
}

void ConfigManager::setLibraries(const QVector<LibraryConfig> &libs) {
    QJsonArray libArray;
    for (const LibraryConfig &lib: libs) {
        QJsonObject obj;
        obj["path"] = lib.path;
        obj["hotkey"] = lib.hotkey;
        obj["enabled"] = lib.enabled;
        libArray.append(obj);
    }
    m_config["libraries"] = libArray;
    emit configChanged();
}

void ConfigManager::addLibrary(const LibraryConfig &lib) {
    auto libs = getLibraries();
    libs.append(lib);
    setLibraries(libs);
}

void ConfigManager::removeLibrary(int index) {
    auto libs = getLibraries();
    if (index >= 0 && index < libs.size()) {
        libs.removeAt(index);
        setLibraries(libs);
    }
}

QJsonObject ConfigManager::getDefaultConfig() {
    QJsonObject config;

    config["libraryPath"] = "";
    config["port"] = 8868;

    QJsonObject shortcuts;
    shortcuts["useHotkey"] = true;
    shortcuts["hotkey"] = "Ctrl+Shift+E";
    config["shortcuts"] = shortcuts;

    QJsonArray libraries;
    config["libraries"] = libraries;

    QJsonArray windowPos = {900, 50};
    QJsonArray windowSize = {540, 430};
    config["windowPosition"] = windowPos;
    config["windowSize"] = windowSize;

    QJsonObject ui;
    ui["categoryButtonSize"] = 90;
    ui["gridCellSize"] = 120;
    ui["gridColumns"] = 3;
    config["ui"] = ui;

    QJsonObject behavior;
    behavior["copyOnDoubleClick"] = true;
    behavior["highlightOnClick"] = true;
    behavior["searchDelayMs"] = 300;
    config["behavior"] = behavior;

    QJsonObject performance;
    performance["thumbnailCacheSize"] = 200;
    performance["lazyLoadEnabled"] = true;
    config["performance"] = performance;

    return config;
}

QString ConfigManager::getLibraryPath() const {
    auto libs = getLibraries();
    return libs.isEmpty() ? "" : libs.first().path;
}

void ConfigManager::setLibraryPath(const QString &path) {
    auto libs = getLibraries();
    if (libs.isEmpty()) {
        LibraryConfig newLib(path, "Ctrl+Shift+E", true);
        addLibrary(newLib);
    } else {
        libs[0].path = path;
        setLibraries(libs);
    }
}

QString ConfigManager::getHotkey() const {
    auto libs = getLibraries();
    return libs.isEmpty() ? "Ctrl+Shift+E" : libs.first().hotkey;
}

bool ConfigManager::isUseHotkey() const {
    auto libs = getLibraries();
    return libs.isEmpty() ? true : libs.first().enabled;
}

void ConfigManager::setHotkey(const QString &hotkey) {
    auto libs = getLibraries();
    if (libs.isEmpty()) {
        LibraryConfig newLib("", hotkey, true);
        addLibrary(newLib);
    } else {
        libs[0].hotkey = hotkey;
        setLibraries(libs);
    }
}

int ConfigManager::getPort() const {
    return m_config["port"].toInt();
}

QSize ConfigManager::getWindowSize() const {
    QJsonArray sizeArray = m_config["windowSize"].toArray();
    if (sizeArray.size() == 2) {
        return QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
    }
    return QSize(540, 430);
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

QString ConfigManager::getConfigPath() const {
    return m_configPath;
}