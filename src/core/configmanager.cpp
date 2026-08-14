#include "configmanager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QSaveFile>
#include <algorithm>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent) {
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir configDir = QDir(exeDir + "/.stickersmanager");
    QDir().mkpath(configDir.absolutePath());
    m_configPath = configDir.absolutePath() + "/config.json";
    m_settingsPath = configDir.absolutePath() + "/settings.json";

    if (!loadConfig()) {
        qWarning() << "Using default configuration";
    }
    loadSettings();
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
    QSaveFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(m_config);
    if (file.write(doc.toJson()) == -1 || !file.commit()) {
        return false;
    }

    return true;
}

void ConfigManager::setConfig(const QJsonObject &cfg) {
    m_config = cfg;
}

QJsonObject ConfigManager::config() const {
    return m_config;
}

bool ConfigManager::loadSettings() {
    QFile file(m_settingsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_settings = QJsonObject();
        m_settings["doubleClickTarget"] = QString("settings");
        m_settings["searchDelayMs"] = 300;
        m_settings["thumbnailCacheSize"] = 200;
        m_settings["checkForUpdatesOnStartup"] = true;
        m_settings["startWithWindows"] = false;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        m_settings = QJsonObject();
        m_settings["doubleClickTarget"] = QString("settings");
        m_settings["checkForUpdatesOnStartup"] = true;
        m_settings["startWithWindows"] = false;
        return false;
    }

    m_settings = doc.object();
    return true;
}

bool ConfigManager::saveSettings() {
    QSaveFile file(m_settingsPath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument doc(m_settings);
    if (file.write(doc.toJson()) == -1 || !file.commit())
        return false;

    return true;
}

QString ConfigManager::getDoubleClickTarget() const {
    return m_settings["doubleClickTarget"].toString("settings");
}

void ConfigManager::setDoubleClickTarget(const QString &v) {
    m_settings["doubleClickTarget"] = v;
}

bool ConfigManager::getCheckForUpdatesOnStartup() const {
    return m_settings["checkForUpdatesOnStartup"].toBool(true);
}

void ConfigManager::setCheckForUpdatesOnStartup(bool v) {
    m_settings["checkForUpdatesOnStartup"] = v;
}

bool ConfigManager::getStartWithWindows() const {
    return m_settings["startWithWindows"].toBool(false);
}

void ConfigManager::setStartWithWindows(bool v) {
    m_settings["startWithWindows"] = v;
}

QJsonObject ConfigManager::libCatSettings(const LibraryConfig &lib, const QString &category) {
    return lib.settings[category].toObject();
}

QVector<LibraryConfig> ConfigManager::getLibraries() const {
    QVector<LibraryConfig> result;
    QJsonArray libArray = m_config["libraries"].toArray();
    result.reserve(libArray.size());
    for (const QJsonValue &val: libArray) {
        QJsonObject obj = val.toObject();
        LibraryConfig lib;
        lib.id = obj["id"].toInt(-1);
        lib.path = obj["path"].toString();
        lib.hotkey = obj["hotkey"].toString();
        lib.enabled = obj["enabled"].toBool(true);
        lib.settings = obj["settings"].toObject();
        result.append(lib);
    }
    // legacy migration: entries without id get their array position
    for (int i = 0; i < result.size(); ++i)
        if (result[i].id < 0)
            result[i].id = i;
    // id is the authoritative order marker
    std::sort(result.begin(), result.end(),
              [](const LibraryConfig &a, const LibraryConfig &b) { return a.id < b.id; });
    return result;
}

void ConfigManager::setLibraries(const QVector<LibraryConfig> &libs) {
    QVector<LibraryConfig> sorted = libs;
    std::sort(sorted.begin(), sorted.end(),
              [](const LibraryConfig &a, const LibraryConfig &b) { return a.id < b.id; });
    QJsonArray libArray;
    for (const LibraryConfig &lib: sorted) {
        QJsonObject obj;
        obj["id"] = lib.id;
        obj["path"] = lib.path;
        obj["hotkey"] = lib.hotkey;
        obj["enabled"] = lib.enabled;
        if (!lib.settings.isEmpty())
            obj["settings"] = lib.settings;
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

    QJsonObject def;

    QJsonObject window;
    QJsonArray windowPos = {900, 50};
    QJsonArray windowSize = {540, 430};
    window["position"] = windowPos;
    window["size"] = windowSize;
    window["alwaysOnTop"] = true;
    def["window"] = window;

    QJsonObject ui;
    ui["categoryButtonSize"] = 90;
    ui["gridCellSize"] = 120;
    ui["gridColumns"] = 3;
    ui["recentLimit"] = 100;
    ui["recentEnabled"] = true;
    def["ui"] = ui;

    QJsonObject behavior;
    behavior["copyOnDoubleClick"] = true;
    behavior["highlightOnClick"] = true;
    behavior["animateThumbnails"] = false;
    behavior["animatePreview"] = true;
    behavior["showFileTypeTag"] = true;
    behavior["showStickerName"] = true;
    behavior["showStickerSize"] = true;
    behavior["showCategoryName"] = true;
    behavior["showCategoryCount"] = true;
    def["behavior"] = behavior;

    QJsonObject performance;
    def["performance"] = performance;

    config["default"] = def;
    return config;
}

QSize ConfigManager::getWindowSize() const {
    QJsonObject window = defaultBlock()["window"].toObject();
    QJsonArray sizeArray = window["size"].toArray();
    if (sizeArray.size() == 2)
        return QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
    return QSize(540, 430);
}

QPoint ConfigManager::getWindowPosition() const {
    QJsonObject window = defaultBlock()["window"].toObject();
    QJsonArray posArray = window["position"].toArray();
    if (posArray.size() == 2)
        return QPoint(posArray[0].toInt(), posArray[1].toInt());
    return QPoint(900, 50);
}

int ConfigManager::getCategoryButtonSize() const {
    return defaultBlock()["ui"].toObject()["categoryButtonSize"].toInt(90);
}

int ConfigManager::getGridCellSize() const {
    return defaultBlock()["ui"].toObject()["gridCellSize"].toInt(120);
}

int ConfigManager::getGridColumns() const {
    return defaultBlock()["ui"].toObject()["gridColumns"].toInt(3);
}

int ConfigManager::getRecentLimit() const {
    return defaultBlock()["ui"].toObject()["recentLimit"].toInt(100);
}

bool ConfigManager::recentEnabled() const {
    return defaultBlock()["ui"].toObject()["recentEnabled"].toBool(true);
}

int ConfigManager::getThumbnailCacheSize() const {
    return m_settings["thumbnailCacheSize"].toInt(200);
}

void ConfigManager::setThumbnailCacheSize(int v) {
    m_settings["thumbnailCacheSize"] = v;
}

bool ConfigManager::animateThumbnails() const {
    return defaultBlock()["behavior"].toObject()["animateThumbnails"].toBool(false);
}

bool ConfigManager::animatePreview() const {
    return defaultBlock()["behavior"].toObject()["animatePreview"].toBool(true);
}

bool ConfigManager::showFileTypeTag() const {
    return defaultBlock()["behavior"].toObject()["showFileTypeTag"].toBool(true);
}

bool ConfigManager::showStickerName() const {
    return defaultBlock()["behavior"].toObject()["showStickerName"].toBool(true);
}

bool ConfigManager::showStickerSize() const {
    return defaultBlock()["behavior"].toObject()["showStickerSize"].toBool(true);
}

bool ConfigManager::showCategoryName() const {
    return defaultBlock()["behavior"].toObject()["showCategoryName"].toBool(true);
}

bool ConfigManager::showCategoryCount() const {
    return defaultBlock()["behavior"].toObject()["showCategoryCount"].toBool(true);
}

bool ConfigManager::copyOnDoubleClick() const {
    return defaultBlock()["behavior"].toObject()["copyOnDoubleClick"].toBool(true);
}

bool ConfigManager::highlightOnClick() const {
    return defaultBlock()["behavior"].toObject()["highlightOnClick"].toBool(true);
}

int ConfigManager::getSearchDelayMs() const {
    return m_settings["searchDelayMs"].toInt(300);
}

void ConfigManager::setSearchDelayMs(int v) {
    m_settings["searchDelayMs"] = v;
}

bool ConfigManager::getDefaultAlwaysOnTop() const {
    return defaultBlock()["window"].toObject()["alwaysOnTop"].toBool(true);
}

QSize ConfigManager::getEffectiveWindowSize(const LibraryConfig &lib) const {
    QJsonObject win = libCatSettings(lib, "window");
    QJsonArray arr = win["size"].toArray();
    if (arr.size() == 2)
        return QSize(arr[0].toInt(), arr[1].toInt());
    return getWindowSize();
}

QPoint ConfigManager::getEffectiveWindowPosition(const LibraryConfig &lib) const {
    QJsonObject win = libCatSettings(lib, "window");
    QJsonArray arr = win["position"].toArray();
    if (arr.size() == 2)
        return QPoint(arr[0].toInt(), arr[1].toInt());
    return getWindowPosition();
}

int ConfigManager::getEffectiveCategoryButtonSize(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "ui");
    if (o.contains("categoryButtonSize")) return o["categoryButtonSize"].toInt();
    return getCategoryButtonSize();
}

int ConfigManager::getEffectiveGridCellSize(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "ui");
    if (o.contains("gridCellSize")) return o["gridCellSize"].toInt();
    return getGridCellSize();
}

int ConfigManager::getEffectiveGridColumns(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "ui");
    if (o.contains("gridColumns")) return o["gridColumns"].toInt();
    return getGridColumns();
}

int ConfigManager::getEffectiveRecentLimit(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "ui");
    if (o.contains("recentLimit")) return o["recentLimit"].toInt();
    return getRecentLimit();
}

bool ConfigManager::getEffectiveRecentEnabled(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "ui");
    if (o.contains("recentEnabled")) return o["recentEnabled"].toBool();
    return recentEnabled();
}

int ConfigManager::getEffectiveThumbnailCacheSize(const LibraryConfig &) const {
    return getThumbnailCacheSize();
}

bool ConfigManager::getEffectiveAnimateThumbnails(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("animateThumbnails")) return o["animateThumbnails"].toBool();
    return animateThumbnails();
}

bool ConfigManager::getEffectiveAnimatePreview(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("animatePreview")) return o["animatePreview"].toBool();
    return animatePreview();
}

bool ConfigManager::getEffectiveShowFileTypeTag(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("showFileTypeTag")) return o["showFileTypeTag"].toBool();
    return showFileTypeTag();
}

bool ConfigManager::getEffectiveShowStickerName(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("showStickerName")) return o["showStickerName"].toBool();
    return showStickerName();
}

bool ConfigManager::getEffectiveShowStickerSize(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("showStickerSize")) return o["showStickerSize"].toBool();
    return showStickerSize();
}

bool ConfigManager::getEffectiveShowCategoryName(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("showCategoryName")) return o["showCategoryName"].toBool();
    return showCategoryName();
}

bool ConfigManager::getEffectiveShowCategoryCount(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("showCategoryCount")) return o["showCategoryCount"].toBool();
    return showCategoryCount();
}

bool ConfigManager::getEffectiveHighlightOnClick(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("highlightOnClick")) return o["highlightOnClick"].toBool();
    return highlightOnClick();
}

bool ConfigManager::getEffectiveCopyOnDoubleClick(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "behavior");
    if (o.contains("copyOnDoubleClick")) return o["copyOnDoubleClick"].toBool();
    return copyOnDoubleClick();
}

bool ConfigManager::getEffectiveAlwaysOnTop(const LibraryConfig &lib) const {
    QJsonObject o = libCatSettings(lib, "window");
    if (o.contains("alwaysOnTop")) return o["alwaysOnTop"].toBool();
    return getDefaultAlwaysOnTop();
}

QString ConfigManager::getConfigPath() const {
    return m_configPath;
}
