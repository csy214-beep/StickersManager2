# Change Log

## Latest Changes

### [Bug Fixes and Configuration Safety Update]

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

### [Multi-Window Update]

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
- MODIFIED: main.cpp manages QMap<QString, MainWindow\*> windows
- MODIFIED: main.cpp handles global hotkeys centrally
- MODIFIED: main.cpp connects tray menu signals to window toggling
- MODIFIED: MainWindow::loadLibrary() uses m_libConfig.path
- MODIFIED: MainWindow default constructor uses first library from config
- MODIFIED: ImagePreviewDialog converts QImage to QPixmap
- MODIFIED: All docs updated in docs/.ai/
