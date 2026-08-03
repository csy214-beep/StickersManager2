#include "settingsdialog.h"
#include "generalsettingspage.h"
#include "librarysettingspage.h"
#include "basesettingspage.h"
#include "aboutpage.h"
#include "configmanager.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(ConfigManager *config, bool keepOpenOnSave, QWidget *parent)
    : QDialog(parent), m_config(config), m_keepOpenOnSave(keepOpenOnSave)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowTitle("Settings");
    setMinimumSize(600, 500);

    auto *layout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);

    m_generalPage = new GeneralSettingsPage(config, this);
    m_libraryPage = new LibrarySettingsPage(config, this);
    m_basePage = new BaseSettingsPage(config, this);
    m_aboutPage = new AboutPage(config, this);

    m_tabWidget->addTab(m_generalPage, "General");
    m_tabWidget->addTab(m_libraryPage, "Libraries");
    m_tabWidget->addTab(m_basePage, "Base");
    m_tabWidget->addTab(m_aboutPage, "About");

    layout->addWidget(m_tabWidget);

    auto *saveBtn = new QPushButton("Save && Reload");
    auto *cancelBtn = new QPushButton("Cancel");

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::onSave() {
    m_generalPage->applyToConfig();
    if (!m_libraryPage->collectToConfig())
        return;
    m_basePage->applyToConfig();

    m_config->saveConfig();
    m_config->saveSettings();
    if (m_keepOpenOnSave)
        emit applied();
    else
        accept();
}
