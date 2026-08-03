#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QWidget>

class ConfigManager;
class QLabel;

class AboutPage : public QWidget {
    Q_OBJECT
public:
    explicit AboutPage(ConfigManager *config, QWidget *parent = nullptr);
private:
    static QString formatSize(qint64 bytes);
    qint64 dirSize(const QString &path) const;

    ConfigManager *m_config;
    QLabel *m_storageLabel;
};

#endif // ABOUTPAGE_H
