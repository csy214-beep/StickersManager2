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
    ConfigManager *m_config;
    QLabel *m_storageLabel;
};

#endif // ABOUTPAGE_H
