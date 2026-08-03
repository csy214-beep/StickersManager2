#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class ConfigManager;
class GeneralSettingsPage;
class LibrarySettingsPage;
class BaseSettingsPage;
class AboutPage;
class QTabWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(ConfigManager *config, bool keepOpenOnSave = false, QWidget *parent = nullptr);
signals:
    void applied();
private slots:
    void onSave();
private:
    ConfigManager *m_config;
    bool m_keepOpenOnSave;
    QTabWidget *m_tabWidget;
    GeneralSettingsPage *m_generalPage;
    LibrarySettingsPage *m_libraryPage;
    BaseSettingsPage *m_basePage;
    AboutPage *m_aboutPage;
};

#endif // SETTINGSDIALOG_H
