#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QShortcut>
#include <QGridLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QTimer>
#include <QMap>

#include "configmanager.h"
#include "stickerlibrary.h"
#include "thumbnailcache.h"
#include "stickercell.h"
#include "categorybutton.h"
#include "imagepreviewdialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(ConfigManager *config, const LibraryConfig &libConfig, QWidget *parent = nullptr);
    ~MainWindow();

    void showWindow();
    void applyWindowSettings();

    LibraryConfig getLibraryConfig() const;
    void updateLibraryConfig(const LibraryConfig &lib);
    void applySettings();

public slots:
    void reloadLibrary();
    void performSearch();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSearchTextChanged(const QString &text);
    void onCategorySearchTextChanged(const QString &text);
    void onCategoryClicked();
    void onStickerClicked(const QString &filePath);
    void onStickerDoubleClicked(const QString &filePath);
    void onStickerRightClicked(const QString &filePath);
    void delayedSearch();
    void onThumbnailLoaded(const QString &filePath, const QPixmap &pixmap);
    void handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap);

private:
    void initUI();
    QWidget *createCategoryPanel();
    QWidget *createStickerPanel();
    void loadLibrary();
    void populateCategories();
    void showCategory(const QString &categoryName);
    void displayStickers(const QVector<QString> &stickers);
    void recalculateGridColumns();
    void updateCellVisibility();

    ConfigManager *m_config;
    StickerLibrary *m_library;
    ThumbnailCache *m_thumbnailCache;
    LibraryConfig m_libConfig;

    QWidget *m_categoryPanel = nullptr;
    QScrollArea *m_categoryScroll;
    QWidget *m_categoryContainer;
    QVBoxLayout *m_categoryLayout;
    QScrollArea *m_stickerScroll;
    QWidget *m_stickerContainer;
    QGridLayout *m_gridLayout;
    QLineEdit *m_searchInput;
    QLineEdit *m_categorySearchInput;

    QTimer *m_searchTimer;
    QString m_currentCategory;
    QVector<StickerCell *> m_currentCells;

    QMap<CategoryButton *, QString> m_categoryButtons;
    QMap<QString, CategoryButton *> m_pendingCategoryButtons;
    QMap<QString, StickerCell *> m_cellMap;
};

#endif // MAINWINDOW_H
