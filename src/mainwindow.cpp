#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QKeySequence>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include  "tray.h"
#include  "convertcodetostring.hpp"

MainWindow::MainWindow(ConfigManager *config, QWidget *parent)
    : QMainWindow(parent)
      , m_config(config)
      , m_library(nullptr)
      , m_thumbnailCache(nullptr)
      , m_searchTimer(new QTimer(this)) {
    m_thumbnailCache = new ThumbnailCache(m_config->getThumbnailCacheSize(), this);
    m_library = new StickerLibrary(this);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    // 输入监听器
    listener = new GlobalInputListener();
    // 键盘按键
    connect(listener, &GlobalInputListener::keyReleased, [&](int keyCode, ModifierKeys modifiers) {
        QString keyName = keyCodeToKeyString(keyCode);
        QString modifiersName = modifiersToString(modifiers);
        QString hotkey = modifiersName + "+" + keyName;
        if (!modifiers)hotkey = keyName;
        // 自定义全局快捷键窗口显示
        if (ShortcutCompare::compareShortcutKeys(hotkey, m_config->getHotkey())) {
            if (isHidden()) {
                showWindow();
            } else {
                hide();
            }
        }
    });

    // 连接缩略图信号
    connect(m_thumbnailCache, &ThumbnailCache::thumbnailReady,
            this, &MainWindow::onThumbnailLoaded);

    initUI();
    loadLibrary();
    loadStyle();
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, &QTimer::timeout, this, &MainWindow::delayedSearch);
    // 连接托盘图标信号
    connect(TrayIcon::instance()->action_showWin, &QAction::triggered, this, &MainWindow::showWindow);
    connect(TrayIcon::instance(), &TrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        // 判断是否为双击动作
        if (reason == QSystemTrayIcon::DoubleClick) {
            if (isHidden()) {
                showWindow();
            } else {
                hide();
            }
        }
    });
    connect(TrayIcon::instance()->action_rescan, &QAction::triggered, this, &MainWindow::loadLibrary);
    // 开始监听
    if (m_config->isUseHotkey()) {
        if (!listener->startListening()) {
            qCritical() << "Failed to start global input listening";
        } else {
            qDebug() << "Global input listener is running. ";
        }
    }
}

MainWindow::~MainWindow() {
    delete m_thumbnailCache;
    delete m_library;
}

void MainWindow::loadStyle() {
    QString qssFilePath = ":/assets/window.qss";
    // 打开QSS文件
    QFile file(qssFilePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to open QSS file:" << qssFilePath;
        return;
    }

    // 读取文件内容
    QString styleSheet = QLatin1String(file.readAll());
    file.close();

    // 将样式应用到整个应用程序
    qApp->setStyleSheet(styleSheet);
    setStyleSheet(styleSheet);
    update();
    qDebug() << "Loaded style sheet:" << qssFilePath;
}

void MainWindow::initUI() {
    // (保持你原来的代码)
    setWindowTitle("Stickers Manager");
    QSize windowSize = m_config->getWindowSize();
    QPoint windowPos = m_config->getWindowPosition();
    setGeometry(windowPos.x(), windowPos.y(), windowSize.width(), windowSize.height());
    setWindowFlags(Qt::Window);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    QWidget *leftPanel = createCategoryPanel();
    mainLayout->addWidget(leftPanel, 1);
    QWidget *rightPanel = createStickerPanel();
    mainLayout->addWidget(rightPanel, 3);

    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, &MainWindow::hideWindow);
}

QWidget *MainWindow::createCategoryPanel() {
    QWidget *panel = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    m_categoryScroll = new QScrollArea();
    m_categoryScroll->setWidgetResizable(true);
    m_categoryContainer = new QWidget();
    m_categoryLayout = new QVBoxLayout(m_categoryContainer);
    m_categoryLayout->setAlignment(Qt::AlignTop);
    m_categoryLayout->setSpacing(8);
    m_categoryLayout->setContentsMargins(5, 5, 5, 5);
    m_categoryScroll->setWidget(m_categoryContainer);
    layout->addWidget(m_categoryScroll);
    return panel;
}

QWidget *MainWindow::createStickerPanel() {
    QWidget *panel = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *searchWidget = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search...");
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    QPushButton *clearButton = new QPushButton("✕");
    clearButton->setToolTip("Clear");
    clearButton->setFixedWidth(30);
    connect(clearButton, &QPushButton::clicked, m_searchInput, &QLineEdit::clear);
    searchLayout->addWidget(m_searchInput);
    searchLayout->addWidget(clearButton);
    layout->addWidget(searchWidget);

    m_stickerScroll = new QScrollArea();
    m_stickerScroll->setWidgetResizable(true);
    m_stickerContainer = new QWidget();
    m_gridLayout = new QGridLayout(m_stickerContainer);
    m_gridLayout->setSpacing(8);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_gridLayout->setContentsMargins(5, 5, 5, 5);
    m_stickerScroll->setWidget(m_stickerContainer);
    layout->addWidget(m_stickerScroll);

    return panel;
}

void MainWindow::loadLibrary() {
    // (保持你原来的代码)
    QString libraryPath = m_config->getLibraryPath();
    if (libraryPath.isEmpty() || !QDir(libraryPath).exists()) {
        libraryPath = QFileDialog::getExistingDirectory(this, "Select Stickers Library", QDir::homePath());
        if (libraryPath.isEmpty()) return;
        m_config->setLibraryPath(libraryPath);
        m_config->saveConfig();
    }
    if (m_library->setLibraryPath(libraryPath)) {
        populateCategories();
    }
}

// [重要修改] 修复侧边栏加载逻辑
void MainWindow::populateCategories() {
    // 清理旧的
    QLayoutItem *child;
    while ((child = m_categoryLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    m_categoryButtons.clear();
    m_pendingCategoryButtons.clear(); // 清空待加载列表

    QMap<QString, QVector<QString> > categories = m_library->getCategories();
    int buttonSize = m_config->getCategoryButtonSize();

    for (const QString &categoryName: categories.keys()) {
        QVector<QString> stickers = categories[categoryName];
        if (stickers.isEmpty()) continue;

        QString firstSticker = stickers.first();
        CategoryButton *button = new CategoryButton(categoryName, firstSticker, buttonSize);

        connect(button, &CategoryButton::clicked, this, &MainWindow::onCategoryClicked);
        m_categoryLayout->addWidget(button);
        m_categoryButtons[button] = categoryName;

        // [核心修改] 像处理 StickerCell 一样处理 CategoryButton
        // 1. 尝试从缓存直接拿
        QPixmap cached = m_thumbnailCache->get(firstSticker);
        if (!cached.isNull()) {
            button->setThumbnail(cached);
        } else {
            // 2. 缓存没有，注册到待处理列表，并请求加载
            m_pendingCategoryButtons[firstSticker] = button;
            m_thumbnailCache->loadThumbnailAsync(firstSticker, QSize(buttonSize, buttonSize));
        }
    }
    m_categoryLayout->addStretch();

    if (!categories.isEmpty()) {
        showCategory(categories.keys().first());
    }
}

void MainWindow::showCategory(const QString &categoryName) {
    m_currentCategory = categoryName;
    m_searchInput->clear();
    // 侧边栏的加载不要取消，只取消右侧内容的加载
    // m_thumbnailCache->cancelAllLoads();

    displayStickers(m_library->getCategories().value(categoryName));
}

void MainWindow::displayStickers(const QVector<QString> &stickers) {
    // 这里的 cellMap 清空只针对右侧网格，不影响侧边栏
    m_cellMap.clear();

    QLayoutItem *child;
    while ((child = m_gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    m_currentCells.clear();

    int cellSize = m_config->getGridCellSize();
    int columns = m_config->getGridColumns();

    for (int i = 0; i < stickers.size(); ++i) {
        const QString &stickerPath = stickers[i];
        QFileInfo fileInfo(stickerPath);
        if (fileInfo.fileName().startsWith(".preview")) continue;

        int row = i / columns;
        int col = i % columns;

        StickerCell *cell = new StickerCell(stickerPath, cellSize);
        connect(cell, &StickerCell::clicked, this, &MainWindow::onStickerClicked);
        connect(cell, &StickerCell::doubleClicked, this, &MainWindow::onStickerDoubleClicked);
        cell->setToolTip(fileInfo.fileName());

        m_gridLayout->addWidget(cell, row, col);
        m_currentCells.append(cell);

        // 注册到映射表
        m_cellMap[stickerPath] = cell;
    }

    // 延迟批量加载
    QTimer::singleShot(50, [this, cellSize]() {
        QVector<QPair<QString, QSize> > thumbnailsToLoad;
        // 使用略小于单元格的大小，防止撑满
        QSize targetSize(cellSize - 10, cellSize - 10);
        for (StickerCell *cell: m_currentCells) {
            QString filePath = cell->getFilePath();

            // 先看缓存
            QPixmap cached = m_thumbnailCache->get(filePath);
            if (!cached.isNull()) {
                cell->setThumbnail(cached);
            } else {
                thumbnailsToLoad.append(qMakePair(filePath, targetSize));
            }
        }
        if (!thumbnailsToLoad.isEmpty()) {
            m_thumbnailCache->loadThumbnailsAsync(thumbnailsToLoad);
        }
    });
}

void MainWindow::onThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    // 确保在主线程更新UI
    QMetaObject::invokeMethod(this, [this, filePath, pixmap]() {
        handleThumbnailLoaded(filePath, pixmap);
    }, Qt::QueuedConnection);
}

// [核心修改] 统一处理回调
void MainWindow::handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    if (pixmap.isNull()) return;

    // 1. 检查是不是右侧网格的图片
    if (m_cellMap.contains(filePath)) {
        StickerCell *cell = m_cellMap[filePath];
        if (cell) {
            cell->setThumbnail(pixmap);
        }
    }

    // 2. 检查是不是左侧侧边栏的图片
    if (m_pendingCategoryButtons.contains(filePath)) {
        CategoryButton *btn = m_pendingCategoryButtons[filePath];
        if (btn) {
            btn->setThumbnail(pixmap);
        }
        // 加载完后可以移除，或者保留以防刷新
        m_pendingCategoryButtons.remove(filePath);
    }
}

void MainWindow::copyToClipboard(const QString &filePath) {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(filePath));
    mimeData->setUrls(urls);
    mimeData->setText(filePath);
    QApplication::clipboard()->setMimeData(mimeData);
    hideWindow();
}

void MainWindow::showWindow() {
    if (isHidden()) {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::hideWindow() {
    hide();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (event) {
        event->ignore();
        hideWindow();
    }
}

void MainWindow::reloadLibrary() {
    qDebug() << "Reloading library...";
    if (m_library) {
        if (m_library->scanLibrary()) {
            populateCategories();
        }
    }
}

void MainWindow::performSearch() {
    QString keyword = m_searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        if (!m_currentCategory.isEmpty()) showCategory(m_currentCategory);
        return;
    }
    displayStickers(m_library->searchStickers(keyword));
}

void MainWindow::onSearchTextChanged(const QString &text) {
    if (text.isEmpty()) {
        if (!m_currentCategory.isEmpty()) showCategory(m_currentCategory);
    } else {
        m_searchTimer->stop();
        m_searchTimer->start(300);
    }
}

void MainWindow::onCategoryClicked() {
    CategoryButton *button = qobject_cast<CategoryButton *>(sender());
    if (button && m_categoryButtons.contains(button)) {
        showCategory(m_categoryButtons[button]);
    }
}

void MainWindow::onStickerClicked(const QString &filePath) {
    for (StickerCell *cell: m_currentCells) {
        if (cell->getFilePath() != filePath) {
            cell->clearHighlight();
        }
    }
}

void MainWindow::onStickerDoubleClicked(const QString &filePath) {
    copyToClipboard(filePath);
}

void MainWindow::delayedSearch() {
    performSearch();
}
