# Feature Implementation Details

## 1. Multi-Library Multi-Window

Files: `configmanager.h/cpp`, `mainwindow.h/cpp`, `main.cpp`, `tray.h/cpp`

- `LibraryConfig` struct holds path, hotkey, enabled
- `main.cpp` creates one `MainWindow` per enabled library, keyed by path in `QMap`
- Each `MainWindow` has its own `StickerLibrary` and `ThumbnailCache`
- Window title: `"Stickers Manager - <dirName>"`
- Rescan hot-reload: calls `config.reloadFromDisk()` → creates windows for new library paths, rebuilds hotkey map + tray menu, rescans existing windows

## 2. Global Hotkey Management

Files: `main.cpp`, `convertcodetostring.hpp/cpp`, `globalinputlistener.h/cpp`

- `GlobalInputListener` created once in `main.cpp`, installs `WH_KEYBOARD_LL` hook
- Hotkey-to-window map built by `rebuildHotkeyMapping()` — called at startup and on Rescan
- Each library's `hotkey` string toggles its window visibility
- Case-insensitive, space-insensitive, order-insensitive comparison via `ShortcutCompare`

## 3. Tray Icon Show Submenu

Files: `tray.h/cpp`, `main.cpp`

- `TrayIcon` singleton with `showSubMenu` (QMenu) populated per-library
- `updateShowMenu(libraries)` rebuilds submenu items
- `QMenu::triggered` connected to toggle window by path
- Also updated on Rescan hot-reload

## 4. Image Preview Dialog

Files: `imagepreviewdialog.h/cpp`, `stickercell.h/cpp`, `mainwindow.h/cpp`

- Frameless window with dark background, 80% screen scaling
- Click anywhere or press Escape/Enter/Space to close
- `StickerCell::rightClicked` → `MainWindow::onStickerRightClicked` toggles preview
- Static pointer tracks current preview instance for toggle behavior

## 5. Responsive Layout

Files: `mainwindow.h/cpp`

- `resizeEvent` → `recalculateGridColumns()`
- Columns = max(config minColumns, scrollWidth / (cellSize + spacing))
- Re-displays stickers when column count changes

## 6. Thumbnail Caching

Files: `thumbnailcache.h/cpp`, `asyncthumbnailloader.h/cpp`

- Async loading via `QtConcurrent` + `QThreadPool` (max 2 threads)
- LRU `QCache` with configurable size
- `thumbnailReady` signal consumed by both `StickerCell` and `CategoryButton`
- Pending requests tracked in `QSet<QString>` to avoid duplicate loads
- Each `MainWindow` has its own `ThumbnailCache` instance

## 7. Sticker Copy to Clipboard

Files: `stickercell.cpp`

- Double-click copies sticker to clipboard via `QMimeData` (URLs + text fallback)
- Verifies copy success with `QTimer::singleShot` (50ms)
- Shows tray notification on success
- Right double-click does NOT trigger copy or close
