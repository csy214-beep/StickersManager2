#include "librarysettingspage.h"
#include "configmanager.h"
#include "hotkeycapture.h"
#include "convertcodetostring.hpp"

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
#include <QStyle>
#include <QMessageBox>
#include <QSize>

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

static QSpinBox *makePlainSpinBox(int min, int max, int val, QWidget *parent) {
    auto *sb = new QSpinBox(parent);
    sb->setRange(min, max);
    sb->setValue(val);
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
        int idx = i;
        auto *hkRow = new QHBoxLayout;
        auto *hotkeyBtn = new HotkeyCaptureButton;
        hotkeyBtn->setConflictChecker([this, idx](const QString &hk) {
            if (hk.isEmpty()) return false;
            for (int j = 0; j < m_libs.size(); ++j) {
                if (j == idx || !m_libs[j].hotkeyBtn) continue;
                if (ShortcutCompare::compareShortcutKeys(hk, m_libs[j].hotkeyBtn->hotkey()))
                    return true;
            }
            return false;
        });
        hotkeyBtn->setHotkey(libCfg.hotkey);
        auto *enabledCheck = new QCheckBox("Enabled");
        enabledCheck->setChecked(libCfg.enabled);
        auto *delBtn = new QPushButton("Delete");
        hkRow->addWidget(new QLabel("Hotkey:"));
        hkRow->addWidget(hotkeyBtn, 1);
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
        QJsonObject winSettings = libCfg.settings["window"].toObject();
        QPoint pos = winSettings["position"].toArray().size() == 2
                         ? QPoint(winSettings["position"].toArray()[0].toInt(),
                                  winSettings["position"].toArray()[1].toInt())
                         : m_config->getWindowPosition();
        QSize sz = winSettings["size"].toArray().size() == 2
                       ? QSize(winSettings["size"].toArray()[0].toInt(),
                               winSettings["size"].toArray()[1].toInt())
                       : m_config->getWindowSize();
        bool useCustom = winSettings.contains("customGeometry")
                             ? winSettings["customGeometry"].toBool()
                             : false;
        auto *useCustomGeometry = new QCheckBox("Use custom geometry", this);
        useCustomGeometry->setChecked(useCustom);
        auto *winPosX = makePlainSpinBox(-9999, 9999, pos.x(), this);
        auto *winPosY = makePlainSpinBox(-9999, 9999, pos.y(), this);
        auto *winW = makePlainSpinBox(0, 9999, sz.width(), this);
        auto *winH = makePlainSpinBox(0, 9999, sz.height(), this);
        for (QSpinBox *sb : {winPosX, winPosY, winW, winH})
            sb->setEnabled(useCustom);
        connect(useCustomGeometry, &QCheckBox::toggled, this, [winPosX, winPosY, winW, winH](bool checked) {
            for (QSpinBox *sb : {winPosX, winPosY, winW, winH})
                sb->setEnabled(checked);
        });
        auto *alwaysOnTop = new QComboBox(this);
        alwaysOnTop->addItems(BOOL_ITEMS);
        QString aot = winSettings.contains("alwaysOnTop")
                          ? boolToCombo(winSettings["alwaysOnTop"].toBool())
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

        winOvForm->addRow(useCustomGeometry);
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
        auto *animThumbRow = new QWidget(this);
        auto *animThumbLayout = new QHBoxLayout(animThumbRow);
        animThumbLayout->setContentsMargins(0, 0, 0, 0);
        auto *animThumbInfoBtn = new QPushButton(animThumbRow);
        animThumbInfoBtn->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        animThumbInfoBtn->setIconSize(QSize(16, 16));
        animThumbInfoBtn->setFixedSize(20, 20);
        animThumbInfoBtn->setFlat(true);
        animThumbInfoBtn->setCursor(Qt::PointingHandCursor);
        connect(animThumbInfoBtn, &QPushButton::clicked, this, []() {
            QMessageBox::information(nullptr, "Animate Thumbnails",
                "Enabling this may cause lag or stutter when the library contains "
                "many animated files. It is not recommended to enable it when there "
                "are too many animated files.");
        });
        animThumbLayout->addWidget(ovAnimThumb, 1);
        animThumbLayout->addWidget(animThumbInfoBtn, 0, Qt::AlignLeft);
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
        bhvOvForm->addRow("Animate Thumbnails:", animThumbRow);
        bhvOvForm->addRow("Animate Preview:", ovAnimPrev);
        bhvOvForm->addRow("Show File Type Tag:", ovTag);
        ovLayout->addWidget(bhvOv);

        auto *resetBtn = new QPushButton("Reset", this);
        connect(resetBtn, &QPushButton::clicked, this, [this, useCustomGeometry, winPosX, winPosY, winW, winH,
                                                        alwaysOnTop, ovGridCellSize, ovCategoryBtnSize,
                                                        ovCopyDbl, ovHighlight, ovAnimThumb, ovAnimPrev, ovTag]() {
            useCustomGeometry->setChecked(false);
            QPoint defPos = m_config->getWindowPosition();
            QSize defSize = m_config->getWindowSize();
            winPosX->setValue(defPos.x());
            winPosY->setValue(defPos.y());
            winW->setValue(defSize.width());
            winH->setValue(defSize.height());
            alwaysOnTop->setCurrentIndex(0);
            ovGridCellSize->setValue(0);
            ovCategoryBtnSize->setValue(0);
            for (QComboBox *cb : {ovCopyDbl, ovHighlight, ovAnimThumb, ovAnimPrev, ovTag})
                cb->setCurrentIndex(0);
        });
        ovLayout->addWidget(resetBtn);

        overrideWidget->setVisible(false);

        LibraryEditWidgets w;
        w.pathEdit = pathEdit;
        w.hotkeyBtn = hotkeyBtn;
        w.enabledCheck = enabledCheck;
        w.useCustomGeometry = useCustomGeometry;
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

        connect(hotkeyBtn, &HotkeyCaptureButton::hotkeyChanged, this, &LibrarySettingsPage::validateAllHotkeys);
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
    validateAllHotkeys();
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
    lib.hotkey = w.hotkeyBtn->hotkey();
    lib.enabled = w.enabledCheck->isChecked();

    QJsonObject settings;

    // Window
    QJsonObject win;
    win["customGeometry"] = w.useCustomGeometry->isChecked();
    if (w.useCustomGeometry->isChecked()) {
        QJsonArray pos = {w.winPosX->value(), w.winPosY->value()};
        QJsonArray sz = {w.winW->value(), w.winH->value()};
        win["position"] = pos;
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

void LibrarySettingsPage::validateAllHotkeys() {
    for (int i = 0; i < m_libs.size(); ++i) {
        bool conflict = false;
        if (m_libs[i].hotkeyBtn) {
            const QString hk = m_libs[i].hotkeyBtn->hotkey();
            if (!hk.isEmpty()) {
                for (int j = 0; j < m_libs.size(); ++j) {
                    if (i == j || !m_libs[j].hotkeyBtn) continue;
                    if (ShortcutCompare::compareShortcutKeys(hk, m_libs[j].hotkeyBtn->hotkey())) {
                        conflict = true;
                        break;
                    }
                }
            }
        }
        if (m_libs[i].hotkeyBtn)
            m_libs[i].hotkeyBtn->setConflict(conflict);
    }
}

bool LibrarySettingsPage::collectToConfig() {
    QVector<LibraryConfig> libs;
    libs.reserve(m_libs.size());
    for (int i = 0; i < m_libs.size(); ++i)
        libs.append(collectOne(i));

    for (int i = 0; i < libs.size(); ++i) {
        if (libs[i].hotkey.isEmpty()) continue;
        for (int j = i + 1; j < libs.size(); ++j) {
            if (libs[j].hotkey.isEmpty()) continue;
            if (ShortcutCompare::compareShortcutKeys(libs[i].hotkey, libs[j].hotkey)) {
                QMessageBox::warning(this, "Duplicate Hotkey",
                    QString("Libraries \"%1\" and \"%2\" share the same hotkey: %3\n"
                            "Change one of them before saving.")
                        .arg(QFileInfo(libs[i].path).fileName(),
                             QFileInfo(libs[j].path).fileName(),
                             libs[i].hotkey));
                return false;
            }
        }
    }

    m_config->setLibraries(libs);
    return true;
}
