#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <QWidget>

class ConfigManager;
class QSpinBox;
class QCheckBox;

class GeneralSettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit GeneralSettingsPage(ConfigManager *config, QWidget *parent = nullptr);
    void applyToConfig();
private slots:
    void resetToDefaults();
private:
    ConfigManager *m_config;

    QSpinBox *m_categoryButtonSize;
    QSpinBox *m_gridCellSize;
    QSpinBox *m_recentLimit;
    QCheckBox *m_copyOnDblClick;
    QCheckBox *m_highlightOnClick;
    QCheckBox *m_animateThumbnails;
    QCheckBox *m_animatePreview;
    QCheckBox *m_showFileTypeTag;
    QCheckBox *m_showStickerName;
    QCheckBox *m_showStickerSize;
    QCheckBox *m_showCategoryName;
    QCheckBox *m_showCategoryCount;
    QSpinBox *m_winPosX, *m_winPosY, *m_winW, *m_winH;
    QCheckBox *m_alwaysOnTop;
};

#endif // GENERALSETTINGSPAGE_H
