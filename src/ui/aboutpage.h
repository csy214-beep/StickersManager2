#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QWidget>

class ConfigManager;
class QLabel;
class QPushButton;

class AboutPage : public QWidget {
    Q_OBJECT
public:
    explicit AboutPage(ConfigManager *config, QWidget *parent = nullptr);
private slots:
    void checkForUpdates();
private:
    static QString formatSize(qint64 bytes);
    static int compareVersions(const QString &a, const QString &b);
    qint64 dirSize(const QString &path) const;

    ConfigManager *m_config;
    QPushButton *m_checkUpdateBtn;
    QLabel *m_statusLabel;
    QLabel *m_storageLabel;
};

#endif // ABOUTPAGE_H
