#include "basesettingspage.h"
#include "configmanager.h"
#include "updatechecker.h"
#include "appinfo.h"
#include "launcher.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QFileInfo>
#include <QDir>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

BaseSettingsPage::BaseSettingsPage(ConfigManager *config, QWidget *parent)
    : QWidget(parent), m_config(config)
{
    auto *root = new QVBoxLayout(this);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);

    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);

    auto *trayGroup = new QGroupBox("Tray");
    auto *trayForm = new QFormLayout(trayGroup);

    m_doubleClickTarget = new QComboBox(this);
    populateTargets();
    connect(m_doubleClickTarget, &QComboBox::currentIndexChanged, this, [this](int) {
        // update in memory on change, save is handled by dialog's onSave
    });

    trayForm->addRow("Double-click action:", m_doubleClickTarget);
    contentLayout->addWidget(trayGroup);

    // --- Performance ---
    auto *perfGroup = new QGroupBox("Performance");
    auto *perfForm = new QFormLayout(perfGroup);
    m_searchDelayMs = new QSpinBox(this);
    m_searchDelayMs->setRange(0, 5000);
    m_searchDelayMs->setSuffix(" ms");
    m_searchDelayMs->setValue(config->getSearchDelayMs());
    m_thumbnailCacheSize = new QSpinBox(this);
    m_thumbnailCacheSize->setRange(10, 5000);
    m_thumbnailCacheSize->setValue(config->getThumbnailCacheSize());
    perfForm->addRow("Search Delay:", m_searchDelayMs);
    perfForm->addRow("Thumbnail Cache Size:", m_thumbnailCacheSize);
    contentLayout->addWidget(perfGroup);

    // --- Update ---
    auto *updateGroup = new QGroupBox("Update");
    auto *updateGroupLayout = new QVBoxLayout(updateGroup);
    auto *updateForm = new QFormLayout;
    updateGroupLayout->addLayout(updateForm);
    m_checkOnStartup = new QCheckBox(this);
    m_checkOnStartup->setChecked(config->getCheckForUpdatesOnStartup());
    updateForm->addRow("Check for updates on startup:", m_checkOnStartup);

    m_checkStatus = new QLabel(this);
    m_checkStatus->setWordWrap(true);
    m_checkStatus->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_checkStatus->hide();
    m_checkForUpdatesBtn = new QPushButton("Check for Updates", this);
    updateGroupLayout->addWidget(m_checkForUpdatesBtn);
    updateGroupLayout->addWidget(m_checkStatus);
    contentLayout->addWidget(updateGroup);

    connect(m_checkForUpdatesBtn, &QPushButton::clicked, this, &BaseSettingsPage::checkForUpdates);

    updateCheckStatus();

    contentLayout->addStretch();

    scrollArea->setWidget(content);
    root->addWidget(scrollArea);
}

void BaseSettingsPage::populateTargets() {
    m_doubleClickTarget->clear();
    m_doubleClickTarget->addItem("Show first library", "first-library");
    m_doubleClickTarget->addItem("Show settings", "settings");

    // add each library dir name
    auto libs = m_config->getLibraries();
    for (const auto &lib : libs) {
        QString name = QFileInfo(lib.path).fileName();
        if (!name.isEmpty())
            m_doubleClickTarget->addItem(name, lib.path);
    }

    // select current
    QString current = m_config->getDoubleClickTarget();
    for (int i = 0; i < m_doubleClickTarget->count(); ++i) {
        if (m_doubleClickTarget->itemData(i).toString() == current ||
            m_doubleClickTarget->itemText(i) == current) {
            m_doubleClickTarget->setCurrentIndex(i);
            break;
        }
    }
}

void BaseSettingsPage::updateCheckStatus() {
    if (!m_config->getCheckForUpdatesOnStartup()) {
        m_checkStatus->hide();
        return;
    }

    const UpdateChecker::Result &r = UpdateChecker::lastResult;

    if (!r.success && r.error.isEmpty()) {
        // never checked yet (e.g. settings opened before the startup check finished)
        auto *checker = new UpdateChecker(this);
        connect(checker, &UpdateChecker::finished, this, [this, checker](bool, const QString &, const QString &) {
            checker->deleteLater();
            updateCheckStatus();
        });
        checker->check();
        m_checkStatus->setText("Checking...");
        m_checkStatus->setStyleSheet(QString());
        m_checkStatus->show();
        return;
    }

    if (!r.success) return; // silent on failure

    bool newer = UpdateChecker::compareVersions(r.latestVersion, AppInfo::version()) > 0;
    if (newer) {
        m_checkStatus->setText("Update available: " + r.latestVersion);
        m_checkStatus->setStyleSheet("color: #ff9800;");
    } else {
        m_checkStatus->setText("You are running the latest version: " + AppInfo::version());
        m_checkStatus->setStyleSheet("color: #4caf50;");
    }
    m_checkStatus->show();
}

void BaseSettingsPage::checkForUpdates() {
    m_checkStatus->setText("Checking...");
    m_checkStatus->setStyleSheet(QString());
    m_checkStatus->show();
    m_checkForUpdatesBtn->setEnabled(false);

    auto *checker = new UpdateChecker(this);
    connect(checker, &UpdateChecker::finished, this, [this, checker](bool success, const QString &latestVersion, const QString &error) {
        checker->deleteLater();
        m_checkForUpdatesBtn->setEnabled(true);

        if (!success) {
            m_checkStatus->setText("Error: " + error);
            m_checkStatus->setStyleSheet("color: #d32f2f;");
            return;
        }

        int cmp = UpdateChecker::compareVersions(latestVersion, AppInfo::version());
        if (cmp <= 0) {
            QMessageBox::information(this, "Up to Date",
                                     "You are running the latest version: " + AppInfo::version());
        } else {
            auto result = QMessageBox::question(this, "Update Available",
                "Current version: " + AppInfo::version() + "\n"
                "Latest version: " + latestVersion + "\n\n"
                "Open release page?",
                QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes)
                launch(AppInfo::repoUrl() + "/releases/tag/" + latestVersion);
        }
        updateCheckStatus();
    });
    checker->check();
}

void BaseSettingsPage::applyToConfig() {
    QString val = m_doubleClickTarget->currentData().toString();
    if (val.isEmpty())
        val = m_doubleClickTarget->currentText();
    m_config->setDoubleClickTarget(val);
    m_config->setSearchDelayMs(m_searchDelayMs->value());
    m_config->setThumbnailCacheSize(m_thumbnailCacheSize->value());
    m_config->setCheckForUpdatesOnStartup(m_checkOnStartup->isChecked());
}
