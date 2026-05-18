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
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::reloadFromDisk() {
    bool ok = loadConfig();
    if (!ok) {
        qWarning() << "Using default configuration after reload";
    }
    return ok;
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

    QJsonArray libraries;
    config["libraries"] = libraries;

    QJsonObject window;
    QJsonArray windowPos = {900, 50};
    QJsonArray windowSize = {540, 430};
    window["position"] = windowPos;
    window["size"] = windowSize;
    config["window"] = window;

    QJsonObject ui;
    ui["categoryButtonSize"] = 90;
    ui["gridCellSize"] = 120;
    ui["gridColumns"] = 3;
    config["ui"] = ui;

    QJsonObject behavior;
    behavior["copyOnDoubleClick"] = true;
    behavior["highlightOnClick"] = true;
    behavior["searchDelayMs"] = 300;
    behavior["animateThumbnails"] = false;
    behavior["animatePreview"] = false;
    config["behavior"] = behavior;

    QJsonObject performance;
    performance["thumbnailCacheSize"] = 200;
    config["performance"] = performance;

    return config;
}

QSize ConfigManager::getWindowSize() const {
    QJsonObject window = m_config["window"].toObject();
    QJsonArray sizeArray = window["size"].toArray();
    if (sizeArray.size() == 2) {
        return QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
    }
    // fallback: old flat format
    QJsonArray oldSize = m_config["windowSize"].toArray();
    if (oldSize.size() == 2) {
        return QSize(oldSize[0].toInt(), oldSize[1].toInt());
    }
    return QSize(540, 430);
}

QPoint ConfigManager::getWindowPosition() const {
    QJsonObject window = m_config["window"].toObject();
    QJsonArray posArray = window["position"].toArray();
    if (posArray.size() == 2) {
        return QPoint(posArray[0].toInt(), posArray[1].toInt());
    }
    // fallback: old flat format
    QJsonArray oldPos = m_config["windowPosition"].toArray();
    if (oldPos.size() == 2) {
        return QPoint(oldPos[0].toInt(), oldPos[1].toInt());
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

bool ConfigManager::animateThumbnails() const {
    QJsonObject behavior = m_config["behavior"].toObject();
    return behavior["animateThumbnails"].toBool(false);
}

bool ConfigManager::animatePreview() const {
    QJsonObject behavior = m_config["behavior"].toObject();
    return behavior["animatePreview"].toBool(false);
}

QString ConfigManager::getConfigPath() const {
    return m_configPath;
}
