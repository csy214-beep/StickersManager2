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

    // 加载和保存配置
    bool loadConfig();
    bool saveConfig();

    // 多仓库配置
    QVector<LibraryConfig> getLibraries() const;
    void setLibraries(const QVector<LibraryConfig> &libs);
    void addLibrary(const LibraryConfig &lib);
    void removeLibrary(int index);

    // 兼容旧版本的单一仓库接口
    QString getLibraryPath() const;
    void setLibraryPath(const QString &path);
    QString getHotkey() const;
    bool isUseHotkey() const;
    void setHotkey(const QString &hotkey);

    QSize getWindowSize() const;
    void setWindowSize(const QSize &size);

    QPoint getWindowPosition() const;
    void setWindowPosition(const QPoint &pos);

    // 其他配置项
    int getCategoryButtonSize() const;
    int getGridCellSize() const;
    int getGridColumns() const;
    bool getCopyOnDoubleClick() const;
    int getThumbnailCacheSize() const;

    QString getConfigPath() const;

signals:
    void configChanged();

private:
    QJsonObject m_config;
    QString m_configPath;

    QJsonObject getDefaultConfig();
    void migrateToMultiLibrary();
};

#endif // CONFIGMANAGER_H