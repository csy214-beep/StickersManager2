#include "mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QApplication>
#include <QKeySequence>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include "tray.h"

MainWindow::MainWindow(ConfigManager *config, const LibraryConfig &libConfig, QWidget *parent)
    : QMainWindow(parent), m_config(config), m_library(nullptr), m_thumbnailCache(nullptr),
      m_searchTimer(new QTimer(this)), m_libConfig(libConfig) {
    m_thumbnailCache = new ThumbnailCache(m_config->getThumbnailCacheSize(), this);
    m_library = new StickerLibrary(this);

    connect(m_thumbnailCache, &ThumbnailCache::thumbnailReady,
            this, &MainWindow::onThumbnailLoaded);

    initUI();
    loadLibrary();
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, &QTimer::timeout, this, &MainWindow::delayedSearch);
}

MainWindow::~MainWindow() {
    delete m_thumbnailCache;
    delete m_library;
}

LibraryConfig MainWindow::getLibraryConfig() const {
    return m_libConfig;
}

void MainWindow::initUI() {
    setWindowTitle("Stickers Manager " + TrayIcon::instance()->Version);
    QSize windowSize = m_config->getWindowSize();
    QPoint windowPos = m_config->getWindowPosition();
    setGeometry(windowPos.x(), windowPos.y(), windowSize.width(), windowSize.height());
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    QWidget *leftPanel = createCategoryPanel();
    mainLayout->addWidget(leftPanel, 0);
    QWidget *rightPanel = createStickerPanel();
    mainLayout->addWidget(rightPanel, 1);

    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, &MainWindow::hide);
}

QWidget *MainWindow::createCategoryPanel() {
    QWidget *panel = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    int buttonSize = m_config->getCategoryButtonSize();
    panel->setMaximumWidth(buttonSize + 20);
    panel->setMinimumWidth(buttonSize + 20);

    m_categorySearchInput = new QLineEdit();
    m_categorySearchInput->setPlaceholderText("Search...");
    m_categorySearchInput->setClearButtonEnabled(true);
    connect(m_categorySearchInput, &QLineEdit::textChanged, this, &MainWindow::onCategorySearchTextChanged);
    layout->addWidget(m_categorySearchInput);

    m_categoryScroll = new QScrollArea();
    m_categoryScroll->setWidgetResizable(true);
    m_categoryScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_categoryScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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

    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search...");
    m_searchInput->setClearButtonEnabled(true);
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    layout->addWidget(m_searchInput);

    m_stickerScroll = new QScrollArea();
    m_stickerScroll->setWidgetResizable(true);
    m_stickerScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_stickerScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_stickerContainer = new QWidget();
    m_gridLayout = new QGridLayout(m_stickerContainer);
    m_gridLayout->setSpacing(8);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_gridLayout->setContentsMargins(5, 5, 5, 5);
    m_stickerScroll->setWidget(m_stickerContainer);
    connect(m_stickerScroll->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MainWindow::updateCellVisibility);
    layout->addWidget(m_stickerScroll);

    return panel;
}

void MainWindow::loadLibrary() {
    QString libraryPath = m_libConfig.path;
    if (libraryPath.isEmpty() || !QDir(libraryPath).exists()) {
        libraryPath = QFileDialog::getExistingDirectory(this, "Select Stickers Library", QDir::homePath());
        if (libraryPath.isEmpty()) return;
        m_libConfig.path = libraryPath;
        m_config->addLibrary(m_libConfig);
        m_config->saveConfig();
    }
    if (m_library->setLibraryPath(libraryPath)) {
        populateCategories();
    }
}

void MainWindow::populateCategories()
{
    QLayoutItem *child;
    while ((child = m_categoryLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    m_categoryButtons.clear();
    m_pendingCategoryButtons.clear();

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

        QPixmap cached = m_thumbnailCache->get(firstSticker);
        if (!cached.isNull()) {
            button->setThumbnail(cached);
        }
        else
        {
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
    m_categorySearchInput->clear();

    displayStickers(m_library->getCategories().value(categoryName));
    QTimer::singleShot(0, this, &MainWindow::updateCellVisibility);
}

void MainWindow::displayStickers(const QVector<QString> &stickers)
{
    m_cellMap.clear();

    QLayoutItem *child;
    while ((child = m_gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    m_currentCells.clear();

    int cellSize = m_config->getGridCellSize();
    int minColumns = m_config->getGridColumns();
    int spacing = m_gridLayout->spacing();
    int scrollWidth = m_stickerScroll->viewport()->width();

    int calculatedColumns = scrollWidth / (cellSize + spacing);
    if (calculatedColumns < 1)
        calculatedColumns = 1;
    int columns = qMax(minColumns, calculatedColumns);

    for (int i = 0; i < stickers.size(); ++i) {
        const QString &stickerPath = stickers[i];
        QFileInfo fileInfo(stickerPath);
        if (fileInfo.fileName().startsWith(".preview")) continue;

        int row = i / columns;
        int col = i % columns;

        StickerCell *cell = new StickerCell(stickerPath, cellSize);
        cell->setAnimateEnabled(m_config->animateThumbnails());
        cell->setShowTag(m_config->showFileTypeTag());
        connect(cell, &StickerCell::clicked, this, &MainWindow::onStickerClicked);
        connect(cell, &StickerCell::doubleClicked, this, &MainWindow::onStickerDoubleClicked);
        connect(cell, &StickerCell::rightClicked, this, &MainWindow::onStickerRightClicked);
        cell->setToolTip(fileInfo.fileName());

        m_gridLayout->addWidget(cell, row, col);
        m_currentCells.append(cell);
        m_cellMap[stickerPath] = cell;
    }

    QTimer::singleShot(50, [this, cellSize]()
                       {
        QVector<QPair<QString, QSize> > thumbnailsToLoad;
        QSize targetSize(cellSize - 10, cellSize - 10);
        for (StickerCell *cell: m_currentCells) {
            QString filePath = cell->getFilePath();

            QPixmap cached = m_thumbnailCache->get(filePath);
            if (!cached.isNull()) {
                cell->setThumbnail(cached);
            } else {
                thumbnailsToLoad.append(qMakePair(filePath, targetSize));
            }
        }
        if (!thumbnailsToLoad.isEmpty()) {
            m_thumbnailCache->loadThumbnailsAsync(thumbnailsToLoad);
        } });
}

void MainWindow::onThumbnailLoaded(const QString &filePath, const QPixmap &pixmap)
{
    QMetaObject::invokeMethod(this, [this, filePath, pixmap]() {
        handleThumbnailLoaded(filePath, pixmap);
    }, Qt::QueuedConnection);
}

void MainWindow::handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    if (pixmap.isNull()) return;

    if (m_cellMap.contains(filePath)) {
        StickerCell *cell = m_cellMap[filePath];
        if (cell) {
            cell->setThumbnail(pixmap);
        }
    }

    if (m_pendingCategoryButtons.contains(filePath)) {
        CategoryButton *btn = m_pendingCategoryButtons[filePath];
        if (btn) {
            btn->setThumbnail(pixmap);
        }
        m_pendingCategoryButtons.remove(filePath);
    }
}

void MainWindow::showWindow() {
    if (isHidden()) {
        update();
        show();
        recalculateGridColumns();
        raise();
        activateWindow();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    hide();
    event->ignore();
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
    QTimer::singleShot(0, this, &MainWindow::updateCellVisibility);
}

void MainWindow::onSearchTextChanged(const QString &text) {
    if (text.isEmpty()) {
        if (!m_currentCategory.isEmpty()) showCategory(m_currentCategory);
    } else {
        m_searchTimer->stop();
        m_searchTimer->start(300);
    }
}

void MainWindow::onCategorySearchTextChanged(const QString &text) {
    for (auto it = m_categoryButtons.begin(); it != m_categoryButtons.end(); ++it) {
        CategoryButton *button = it.key();
        QString categoryName = it.value();
        if (text.isEmpty() || categoryName.contains(text, Qt::CaseInsensitive)) {
            button->show();
        } else {
            button->hide();
        }
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

void MainWindow::onStickerRightClicked(const QString &filePath)
{
    for (StickerCell *cell: m_currentCells) {
        if (cell->getFilePath() != filePath) {
            cell->clearHighlight();
        }
    }

    static ImagePreviewDialog *currentPreview = nullptr;
    if (currentPreview) {
        currentPreview->accept();
        currentPreview = nullptr;
    } else {
        currentPreview = new ImagePreviewDialog(filePath, m_config->animatePreview(), this);
        connect(currentPreview, &ImagePreviewDialog::finished, [&]() { currentPreview = nullptr; });
        currentPreview->show();
    }
}

void MainWindow::onStickerDoubleClicked(const QString &filePath) {
    hide();
}

void MainWindow::delayedSearch() {
    performSearch();
}

void MainWindow::updateCellVisibility() {
    if (!m_stickerScroll) return;
    QRect viewportRect = m_stickerScroll->viewport()->rect();
    for (StickerCell *cell : m_currentCells) {
        QRect cellRect(cell->mapTo(m_stickerScroll->viewport(), QPoint(0, 0)), cell->size());
        cell->setInViewport(viewportRect.intersects(cellRect));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    recalculateGridColumns();
    updateCellVisibility();
}

void MainWindow::recalculateGridColumns() {
    if (!m_stickerScroll || !m_gridLayout)
        return;

    int cellSize = m_config->getGridCellSize();
    int minColumns = m_config->getGridColumns();
    int spacing = m_gridLayout->spacing();
    int scrollWidth = m_stickerScroll->viewport()->width();

    int calculatedColumns = scrollWidth / (cellSize + spacing);
    if (calculatedColumns < 1)
        calculatedColumns = 1;
    int newColumns = qMax(minColumns, calculatedColumns);

    static int lastColumns = 0;

    if (newColumns != lastColumns && !m_currentCategory.isEmpty()) {
        lastColumns = newColumns;
        if (m_searchInput->text().isEmpty()) {
            showCategory(m_currentCategory);
        } else {
            performSearch();
        }
    }
}
