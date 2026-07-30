#ifndef BASESETTINGSPAGE_H
#define BASESETTINGSPAGE_H

#include <QWidget>

class ConfigManager;
class QComboBox;
class QSpinBox;

class BaseSettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit BaseSettingsPage(ConfigManager *config, QWidget *parent = nullptr);
    void applyToConfig();
private:
    ConfigManager *m_config;
    QComboBox *m_doubleClickTarget;
    QSpinBox *m_searchDelayMs;
    QSpinBox *m_thumbnailCacheSize;
    void populateTargets();
};

#endif // BASESETTINGSPAGE_H
