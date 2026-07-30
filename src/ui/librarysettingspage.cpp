#include "librarysettingspage.h"
#include "configmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QScrollArea>

static const QStringList BOOL_ITEMS = {"General", "On", "Off"};
static const QStringList BOOL_KEYS = {"copyOnDoubleClick", "highlightOnClick",
                                      "animateThumbnails", "animatePreview", "showFileTypeTag"};

static QSpinBox *makeSpinBox(int min, int max, int val, QWidget *parent) {
    auto *sb = new QSpinBox(parent);
    sb->setRange(min, max);
    sb->setValue(val);
    sb->setSpecialValueText("General");
    return sb;
}

static QComboBox *makeBoolCombo(const QString &current, QWidget *parent) {
    auto *cb = new QComboBox(parent);
    cb->addItems(BOOL_ITEMS);
    int idx = BOOL_ITEMS.indexOf(current);
    if (idx < 0) idx = 0;
    cb->setCurrentIndex(idx);
    return cb;
}

static QString boolToCombo(bool val) { return val ? "On" : "Off"; }

static int comboToBool(const QString &s, bool def) {
    if (s == "On") return 1;
    if (s == "Off") return 0;
    return def ? -1 : -1; // -1 means "use default"
}

LibrarySettingsPage::LibrarySettingsPage(ConfigManager *config, QWidget *parent)
    : QWidget(parent), m_config(config)
{
    auto *root = new QVBoxLayout(this);

    auto *addBtn = new QPushButton("+ Add Library");
    addBtn->setFixedWidth(200);
    auto *topLayout = new QHBoxLayout;
    topLayout->addStretch();
    topLayout->addWidget(addBtn);
    topLayout->addStretch();
    root->addLayout(topLayout);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);

    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);
    m_listLayout = new QVBoxLayout;
    contentLayout->addLayout(m_listLayout);
    contentLayout->addStretch();

    scrollArea->setWidget(content);
    root->addWidget(scrollArea);

    connect(addBtn, &QPushButton::clicked, this, &LibrarySettingsPage::addLibrary);

    buildFromConfig();
}

void LibrarySettingsPage::buildFromConfig() {
    m_libs.clear();
    auto libs = m_config->getLibraries();
    for (const auto &lib : libs)
        m_libs.append(LibraryEditWidgets{});
    rebuildList();
}

void LibrarySettingsPage::rebuildList() {
    while (auto *item = m_listLayout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto libs = m_config->getLibraries(); // use original for dir names
    for (int i = 0; i < m_libs.size(); ++i) {
        QString dirName = i < libs.size() ? QFileInfo(libs[i].path).fileName()
                                          : QString("Library %1").arg(i + 1);
        LibraryConfig libCfg = i < libs.size() ? libs[i] : LibraryConfig();

        auto *card = new QGroupBox;
        auto *cardLayout = new QVBoxLayout(card);

        // Row 1: path + browse
        auto *pathRow = new QHBoxLayout;
        auto *pathEdit = new QLineEdit(libCfg.path);
        pathEdit->setPlaceholderText("Path to sticker library...");
        pathEdit->setMinimumWidth(300);
        auto *browseBtn = new QPushButton("Browse");
        pathRow->addWidget(pathEdit);
        pathRow->addWidget(browseBtn);

        // Row 2: hotkey + enabled
        auto *hkRow = new QHBoxLayout;
        auto *hotkeyEdit = new QLineEdit(libCfg.hotkey);
        hotkeyEdit->setPlaceholderText("e.g. Ctrl+Shift+E");
        auto *enabledCheck = new QCheckBox("Enabled");
        enabledCheck->setChecked(libCfg.enabled);
        auto *delBtn = new QPushButton("Delete");
        hkRow->addWidget(new QLabel("Hotkey:"));
        hkRow->addWidget(hotkeyEdit, 1);
        hkRow->addWidget(enabledCheck);
        hkRow->addWidget(delBtn);

        // Overrides toggle
        auto *overrideToggle = new QPushButton("Show Overrides");
        overrideToggle->setCheckable(true);
        overrideToggle->setChecked(false);

        // Override content
        auto *overrideWidget = new QWidget;
        auto *ovLayout = new QVBoxLayout(overrideWidget);
        ovLayout->setContentsMargins(0, 0, 0, 0);

        // -- Window overrides --
        auto *winOv = new QGroupBox("Window");
        auto *winOvForm = new QFormLayout(winOv);
        QPoint pos = libCfg.settings["window"].toObject()["position"].toArray().size() == 2
                         ? QPoint(libCfg.settings["window"].toObject()["position"].toArray()[0].toInt(),
                                  libCfg.settings["window"].toObject()["position"].toArray()[1].toInt())
                         : QPoint(0, 0);
        QSize sz = libCfg.settings["window"].toObject()["size"].toArray().size() == 2
                       ? QSize(libCfg.settings["window"].toObject()["size"].toArray()[0].toInt(),
                               libCfg.settings["window"].toObject()["size"].toArray()[1].toInt())
                       : QSize(0, 0);
        auto *winPosX = makeSpinBox(-9999, 9999, pos.x(), this);
        auto *winPosY = makeSpinBox(-9999, 9999, pos.y(), this);
        auto *winW = makeSpinBox(0, 9999, sz.width(), this);
        auto *winH = makeSpinBox(0, 9999, sz.height(), this);
        auto *alwaysOnTop = new QComboBox(this);
        alwaysOnTop->addItems(BOOL_ITEMS);
        QString aot = libCfg.settings["window"].toObject().contains("alwaysOnTop")
                          ? boolToCombo(libCfg.settings["window"].toObject()["alwaysOnTop"].toBool())
                          : "General";
        int aotIdx = BOOL_ITEMS.indexOf(aot);
        alwaysOnTop->setCurrentIndex(aotIdx < 0 ? 0 : aotIdx);

        auto *posRow = new QHBoxLayout;
        posRow->addWidget(new QLabel("X:"));
        posRow->addWidget(winPosX);
        posRow->addWidget(new QLabel("Y:"));
        posRow->addWidget(winPosY);
        auto *sizeRow = new QHBoxLayout;
        sizeRow->addWidget(new QLabel("W:"));
        sizeRow->addWidget(winW);
        sizeRow->addWidget(new QLabel("H:"));
        sizeRow->addWidget(winH);

        winOvForm->addRow("Position:", posRow);
        winOvForm->addRow("Size:", sizeRow);
        winOvForm->addRow("Always on Top:", alwaysOnTop);
        ovLayout->addWidget(winOv);

        // -- UI overrides --
        auto *uiOv = new QGroupBox("UI");
        auto *uiOvForm = new QFormLayout(uiOv);
        auto *ovGridCellSize = makeSpinBox(0, 400, libCfg.settings["ui"].toObject()["gridCellSize"].toInt(0), this);
        auto *ovCategoryBtnSize = makeSpinBox(0, 300, libCfg.settings["ui"].toObject()["categoryButtonSize"].toInt(0), this);
        uiOvForm->addRow("Grid Cell Size:", ovGridCellSize);
        uiOvForm->addRow("Category Button Size:", ovCategoryBtnSize);
        ovLayout->addWidget(uiOv);

        // -- Behavior overrides --
        auto *bhvOv = new QGroupBox("Behavior");
        auto *bhvOvForm = new QFormLayout(bhvOv);
        auto *ovCopyDbl = makeBoolCombo(
            libCfg.settings["behavior"].toObject().contains("copyOnDoubleClick")
                ? boolToCombo(libCfg.settings["behavior"].toObject()["copyOnDoubleClick"].toBool())
                : "General", this);
        auto *ovHighlight = makeBoolCombo(
            libCfg.settings["behavior"].toObject().contains("highlightOnClick")
                ? boolToCombo(libCfg.settings["behavior"].toObject()["highlightOnClick"].toBool())
                : "General", this);
        auto *ovAnimThumb = makeBoolCombo(
            libCfg.settings["behavior"].toObject().contains("animateThumbnails")
                ? boolToCombo(libCfg.settings["behavior"].toObject()["animateThumbnails"].toBool())
                : "General", this);
        auto *ovAnimPrev = makeBoolCombo(
            libCfg.settings["behavior"].toObject().contains("animatePreview")
                ? boolToCombo(libCfg.settings["behavior"].toObject()["animatePreview"].toBool())
                : "General", this);
        auto *ovTag = makeBoolCombo(
            libCfg.settings["behavior"].toObject().contains("showFileTypeTag")
                ? boolToCombo(libCfg.settings["behavior"].toObject()["showFileTypeTag"].toBool())
                : "General", this);
        bhvOvForm->addRow("Copy on Double-Click:", ovCopyDbl);
        bhvOvForm->addRow("Highlight on Click:", ovHighlight);
        bhvOvForm->addRow("Animate Thumbnails:", ovAnimThumb);
        bhvOvForm->addRow("Animate Preview:", ovAnimPrev);
        bhvOvForm->addRow("Show File Type Tag:", ovTag);
        ovLayout->addWidget(bhvOv);

        overrideWidget->setVisible(false);

        LibraryEditWidgets w;
        w.pathEdit = pathEdit;
        w.hotkeyEdit = hotkeyEdit;
        w.enabledCheck = enabledCheck;
        w.winPosX = winPosX; w.winPosY = winPosY;
        w.winW = winW; w.winH = winH;
        w.alwaysOnTop = alwaysOnTop;
        w.gridCellSize = ovGridCellSize;
        w.categoryButtonSize = ovCategoryBtnSize;
        w.copyOnDblClick = ovCopyDbl;
        w.highlightOnClick = ovHighlight;
        w.animateThumbnails = ovAnimThumb;
        w.animatePreview = ovAnimPrev;
        w.showFileTypeTag = ovTag;
        w.overrideWidget = overrideWidget;
        m_libs[i] = w;

        cardLayout->addLayout(pathRow);
        cardLayout->addLayout(hkRow);
        cardLayout->addWidget(overrideToggle);
        cardLayout->addWidget(overrideWidget);

        int idx = i;
        connect(browseBtn, &QPushButton::clicked, this, [this, idx]() { browseLibrary(idx); });
        connect(delBtn, &QPushButton::clicked, this, [this, idx]() { removeLibrary(idx); });
        connect(overrideToggle, &QPushButton::toggled, this, [this, idx](bool checked) {
            if (idx < m_libs.size() && m_libs[idx].overrideWidget)
                m_libs[idx].overrideWidget->setVisible(checked);
            auto *btn = qobject_cast<QPushButton *>(sender());
            if (btn) btn->setText(checked ? "Hide Overrides" : "Show Overrides");
        });

        m_listLayout->addWidget(card);
    }
}

void LibrarySettingsPage::addLibrary() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Sticker Library Folder", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    if (path.isEmpty()) return;

    QString canonical = QDir(path).canonicalPath();
    auto libs = m_config->getLibraries();
    for (const auto &lib : libs)
        if (QDir(lib.path).canonicalPath() == canonical)
            return;

    QChar letter = QChar(static_cast<char>('E' + qMin(static_cast<int>(libs.size()), 25)));
    QString hk = QString("Ctrl+Shift+%1").arg(letter);
    LibraryConfig lib(path, hk, true);
    libs.append(lib);
    m_config->setLibraries(libs);

    LibraryEditWidgets w = {};
    m_libs.append(w);
    rebuildList();
}

void LibrarySettingsPage::removeLibrary(int index) {
    if (index < 0 || index >= m_libs.size()) return;
    auto libs = m_config->getLibraries();
    if (index < libs.size()) {
        libs.remove(index);
        m_config->setLibraries(libs);
    }
    m_libs.remove(index);
    rebuildList();
}

void LibrarySettingsPage::browseLibrary(int index) {
    if (index < 0 || index >= m_libs.size()) return;
    QString path = QFileDialog::getExistingDirectory(this, "Select Sticker Library Folder",
                                                     m_libs[index].pathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    if (!path.isEmpty()) {
        m_libs[index].pathEdit->setText(path);
    }
}

LibraryConfig LibrarySettingsPage::collectOne(int index) const {
    if (index < 0 || index >= m_libs.size()) return {};

    const auto &w = m_libs[index];
    LibraryConfig lib;
    lib.path = w.pathEdit->text();
    lib.hotkey = w.hotkeyEdit->text();
    lib.enabled = w.enabledCheck->isChecked();

    QJsonObject settings;

    // Window
    QJsonObject win;
    if (w.winPosX->value() != 0 || w.winPosY->value() != 0) {
        QJsonArray pos = {w.winPosX->value(), w.winPosY->value()};
        win["position"] = pos;
    }
    if (w.winW->value() != 0 || w.winH->value() != 0) {
        QJsonArray sz = {w.winW->value(), w.winH->value()};
        win["size"] = sz;
    }
    if (w.alwaysOnTop->currentText() != "General")
        win["alwaysOnTop"] = (w.alwaysOnTop->currentText() == "On");
    if (!win.isEmpty()) settings["window"] = win;

    // UI
    QJsonObject ui;
    if (w.gridCellSize->value() != 0) ui["gridCellSize"] = w.gridCellSize->value();
    if (w.categoryButtonSize->value() != 0) ui["categoryButtonSize"] = w.categoryButtonSize->value();
    if (!ui.isEmpty()) settings["ui"] = ui;

    // Behavior
    QJsonObject bhv;
    auto setBool = [&](QComboBox *cb, const QString &key) {
        if (cb->currentText() != "General")
            bhv[key] = (cb->currentText() == "On");
    };
    setBool(w.copyOnDblClick, "copyOnDoubleClick");
    setBool(w.highlightOnClick, "highlightOnClick");
    setBool(w.animateThumbnails, "animateThumbnails");
    setBool(w.animatePreview, "animatePreview");
    setBool(w.showFileTypeTag, "showFileTypeTag");
    if (!bhv.isEmpty()) settings["behavior"] = bhv;

    lib.settings = settings;
    return lib;
}

void LibrarySettingsPage::collectToConfig() {
    QVector<LibraryConfig> libs;
    for (int i = 0; i < m_libs.size(); ++i)
        libs.append(collectOne(i));
    m_config->setLibraries(libs);
}
