#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QShortcut>
#include <QGridLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QMimeData>
#include <QClipboard>
#include <QMap>

#include "configmanager.h"
#include "stickerlibrary.h"
#include "thumbnailcache.h"
#include "stickercell.h"
#include "categorybutton.h"
#include "globalinputlistener.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(ConfigManager *config, QWidget *parent = nullptr);

    ~MainWindow();

    void showWindow();

    void hideWindow();

public slots:
    void reloadLibrary();

    void performSearch();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSearchTextChanged(const QString &text);

    void onCategoryClicked();

    void onStickerClicked(const QString &filePath);

    void onStickerDoubleClicked(const QString &filePath);

    void delayedSearch();

    void onThumbnailLoaded(const QString &filePath, const QPixmap &pixmap);

    void handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap);

private:
    void loadStyle();

    void initUI();

    QWidget *createCategoryPanel();

    QWidget *createStickerPanel();

    void loadLibrary();

    void populateCategories();

    void showCategory(const QString &categoryName);

    void displayStickers(const QVector<QString> &stickers);

    // 获取缓存或触发异步加载
    void requestThumbnail(const QString &filePath, int size, bool isCategory);

    void copyToClipboard(const QString &filePath);

    ConfigManager *m_config;
    StickerLibrary *m_library;
    ThumbnailCache *m_thumbnailCache;

    QScrollArea *m_categoryScroll;
    QWidget *m_categoryContainer;
    QVBoxLayout *m_categoryLayout;
    QScrollArea *m_stickerScroll;
    QWidget *m_stickerContainer;
    QGridLayout *m_gridLayout;
    QLineEdit *m_searchInput;

    QTimer *m_searchTimer;
    QString m_currentCategory;
    QVector<StickerCell *> m_currentCells;

    QMap<CategoryButton *, QString> m_categoryButtons;

    // [新增] 用于记录侧边栏按钮的请求：文件路径 -> 按钮指针
    QMap<QString, CategoryButton *> m_pendingCategoryButtons;

    QMap<QString, StickerCell *> m_cellMap;

    GlobalInputListener *listener;
};

#endif // MAINWINDOW_H
