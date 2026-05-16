# Change Log

## [Code Cleanup & Config Hot-Reload]

- **ADDED**: Category search input on left sidebar — `QLineEdit` filters category buttons by name, clicking a matching category jumps to it
- **ADDED**: `ConfigManager::reloadFromDisk()` — re-reads config JSON from disk at runtime
- **ADDED**: `rebuildHotkeyMapping()` helper in main.cpp — rebuilds hotkey-to-window map centrally
- **CHANGED**: Rescan button now hot-reloads config: creates `MainWindow` for new libraries, rebuilds hotkey map + tray menu, then rescans existing windows
- **CHANGED**: Config format simplified — removed legacy `libraryPath`, `shortcuts`, `windowPosition`, `windowSize` (flat); window settings now under `window.position`/`window.size`; removed unused `lazyLoadEnabled`; added `version` field. Backward-compat reads old flat keys as fallback.
- **CHANGED**: ConfigManager — removed `migrateToMultiLibrary()`, `removeLibrary()`, `isUseHotkey()`, `setHotkey()`, `setWindowSize()`, `setWindowPosition()`, `getCopyOnDoubleClick()`, `configChanged` signal (all dead code)
- **REMOVED**: Mouse hook from `GlobalInputListener` — `mouseHookProc`, `mouseReleased`/`mouseMoved` signals, `MouseButton` enum, `isListening` flag (unused)
- **REMOVED**: `MainWindow::loadStyle()` — dead code (commented out calls), along with `getLibraryPath()` and `setLibraryConfig()`
- **REMOVED**: `MainWindow` first constructor (without `LibraryConfig`) — unused; all callers pass explicit config
- **REMOVED**: `StickerLibrary::getSupportedFormats()`, `getLibraryPath()`, `getAllStickers()`, `libraryLoaded`/`errorOccurred` signals — all unused
- **REMOVED**: `ThumbnailCache::cancelLoad()`, `createThumbnail()`, `getCacheKey()` — unused
- **REMOVED**: `AsyncThumbnailLoader::cancelLoad()`, `setMaxThreadCount()`, `loadFinished` signal — unused
- **REMOVED**: `ImageLoader::getFormatDescription()`, `setProgressCallback()`, `progressCallback`, entire `#ifdef HAVE_WEBP` block — dead or never-compiled code
- **REMOVED**: `CategoryButton::m_firstStickerPath`, `setButtonSize()` — unused
- **REMOVED**: `TrayIcon::m_silentMode` — always false, never set
- **REMOVED**: Unused includes across multiple files; duplicate `#include <QMimeData>` in stickercell.cpp
- **REMOVED**: `LogLevel::getLogLevel()` — declared but never called
- **REMOVED**: Stale references to `window.qss`, `window2.qss`, `checkSingleInstance.hpp` from docs and README

## [Bug Fixes and Configuration Safety Update]

- **FIXED**: Removed auto-saving config in resizeEvent - program no longer overwrites user config arbitrarily
- **FIXED**: Right-click preview now correctly applies selection border to target cell
- **FIXED**: Right double-click no longer triggers copy or close window behavior
- **FIXED**: Grid column calculation now properly respects config while adapting to window size
- **FIXED**: Window minimum size reduced to respect config's window size
- **ADDED**: First-time setup wizard to guide user to select sticker library
- **MODIFIED**: StickerCell mouse event logic to handle left/right clicks properly
- **MODIFIED**: MainWindow::onStickerRightClicked to clear other cells' highlight
- **MODIFIED**: ConfigManager - only modifies config on first run or if config is corrupted
- **MODIFIED**: displayStickers uses config columns as minimum, adapts to larger windows

## [Multi-Window Update]

- ADDED: Multi-window support - one window per enabled library
- ADDED: MainWindow constructor accepting LibraryConfig
- ADDED: MainWindow::getLibraryPath(), setLibraryConfig(), getLibraryConfig()
- ADDED: MainWindow::m_libConfig member variable
- ADDED: TrayIcon::showSubMenu (QMenu) for library selection
- ADDED: TrayIcon::updateShowMenu() to populate submenu
- REMOVED: TrayIcon::action_showWin (replaced by submenu)
- REMOVED: MainWindow::showWindowForLibrary() (obsolete)
- REMOVED: MainWindow::listener (moved to main.cpp)
- REMOVED: MainWindow::m_currentLibraryPath (obsolete)
- MODIFIED: main.cpp completely rewritten for multi-window management
- MODIFIED: main.cpp creates MainWindow instances in loop
- MODIFIED: main.cpp manages QMap<QString, MainWindow*> windows
- MODIFIED: main.cpp handles global hotkeys centrally
- MODIFIED: main.cpp connects tray menu signals to window toggling
- MODIFIED: MainWindow::loadLibrary() uses m_libConfig.path
- MODIFIED: MainWindow default constructor uses first library from config
- MODIFIED: ImagePreviewDialog converts QImage to QPixmap
- MODIFIED: All docs updated in docs/.ai/
