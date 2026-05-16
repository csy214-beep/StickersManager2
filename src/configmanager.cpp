#include "configmanager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent) {
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir configDir = QDir(exeDir + "/.stickersmanager");
    QDir().mkpath(configDir.absolutePath());
    m_configPath = configDir.absolutePath() + "/config.json";

    if (!loadConfig()) {
        qWarning() << "Using default configuration";
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
    configFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        m_config = getDefaultConfig();
        return false;
    }

    m_config = doc.object();
    return true;
}

bool ConfigManager::saveConfig() {
    QString tmpPath = m_configPath + ".tmp";
    QFile tmpFile(tmpPath);
    if (!tmpFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QJsonDocument doc(m_config);
    qint64 bytesWritten = tmpFile.write(doc.toJson());
    tmpFile.close();

    if (bytesWritten == -1)
    {
        QFile::remove(tmpPath);
        return false;
    }

    if (QFile::exists(m_configPath))
    {
        if (!QFile::remove(m_configPath))
        {
            QFile::remove(tmpPath);
            return false;
        }
    }

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
        saveConfig();
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
}

void ConfigManager::addLibrary(const LibraryConfig &lib) {
    auto libs = getLibraries();
    libs.append(lib);
    setLibraries(libs);
}

QJsonObject ConfigManager::getDefaultConfig() {
    QJsonObject config;

    config["libraryPath"] = "";

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

QSize ConfigManager::getWindowSize() const {
    QJsonArray sizeArray = m_config["windowSize"].toArray();
    if (sizeArray.size() == 2) {
        return QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
    }
    return QSize(540, 430);
}

QPoint ConfigManager::getWindowPosition() const {
    QJsonArray posArray = m_config["windowPosition"].toArray();
    if (posArray.size() == 2) {
        return QPoint(posArray[0].toInt(), posArray[1].toInt());
    }
    return QPoint(900, 50);
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

int ConfigManager::getThumbnailCacheSize() const {
    QJsonObject performance = m_config["performance"].toObject();
    return performance["thumbnailCacheSize"].toInt(200);
}

QString ConfigManager::getConfigPath() const {
    return m_configPath;
}
