#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QPoint>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonArray>
#include <QVector>

struct LibraryConfig
{
    QString path;
    QString hotkey;
    bool enabled;

    LibraryConfig() : enabled(true) {}
    LibraryConfig(const QString &p, const QString &h, bool e = true)
        : path(p), hotkey(h), enabled(e) {}
};

class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    bool loadConfig();
    bool saveConfig();

    QVector<LibraryConfig> getLibraries() const;
    void setLibraries(const QVector<LibraryConfig> &libs);
    void addLibrary(const LibraryConfig &lib);

    QString getLibraryPath() const;
    void setLibraryPath(const QString &path);
    QString getHotkey() const;

    QSize getWindowSize() const;
    QPoint getWindowPosition() const;

    int getCategoryButtonSize() const;
    int getGridCellSize() const;
    int getGridColumns() const;
    int getThumbnailCacheSize() const;

    QString getConfigPath() const;

private:
    QJsonObject m_config;
    QString m_configPath;

    QJsonObject getDefaultConfig();
    void migrateToMultiLibrary();
};

#endif // CONFIGMANAGER_H
