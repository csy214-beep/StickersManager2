#include "basesettingspage.h"
#include "configmanager.h"

#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QFileInfo>
#include <QDir>
#include <QSpinBox>

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

void BaseSettingsPage::applyToConfig() {
    QString val = m_doubleClickTarget->currentData().toString();
    if (val.isEmpty())
        val = m_doubleClickTarget->currentText();
    m_config->setDoubleClickTarget(val);
    m_config->setSearchDelayMs(m_searchDelayMs->value());
    m_config->setThumbnailCacheSize(m_thumbnailCacheSize->value());
}
