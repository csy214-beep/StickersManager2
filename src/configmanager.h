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

class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);

    ~ConfigManager();

    // 加载和保存配置
    bool loadConfig();

    bool saveConfig();

    // 配置项访问器
    QString getLibraryPath() const;

    void setLibraryPath(const QString &path);

    QString getHotkey() const;

    void setHotkey(const QString &hotkey);

    QSize getWindowSize() const;

    void setWindowSize(const QSize &size);

    QPoint getWindowPosition() const;

    void setWindowPosition(const QPoint &pos);

    // 其他配置项...
    int getCategoryButtonSize() const;

    int getGridCellSize() const;

    int getGridColumns() const;

    bool getCopyOnDoubleClick() const;

    int getThumbnailCacheSize() const;

signals:
    void configChanged(); // 添加信号声明

private:
    QJsonObject m_config;
    QString m_configPath;

    QJsonObject getDefaultConfig();
};

#endif // CONFIGMANAGER_H
