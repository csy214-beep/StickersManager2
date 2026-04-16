# Feature Implementation Details

## 0. Bug Fixes and Improvements (Latest)

### Configuration Safety
- **Removed auto-save**: No longer saves window size/position on every resize
- **Respect user config**: Only modifies config during first-time setup or if corrupted
- **Manual edits preserved**: All user changes to config.json are respected

### Right-Click Behavior
- **Selection border**: Right-click now also applies selection border to target cell
- **Clear other highlights**: Right-click clears highlight from other cells
- **Right double-click disabled**: Right double-click no longer triggers copy or window close

### Grid Layout Behavior
- **Config as minimum**: Uses config's gridColumns as minimum column count
- **Adaptive to window**: Increases columns when window is resized larger
- **No forced overwrite**: Window minimum size reduced to respect config's windowSize

### First-Time Setup Wizard
- **Auto-detection**: Detects if no valid sticker library is configured
- **Welcome dialog**: Explains sticker library organization
- **File dialog**: Guides user to select sticker library folder
- **Auto-save config**: Saves selected path with default hotkey Ctrl+Shift+E

## 1. Multi-Library Multi-Window Support

Files: configmanager.h/cpp, mainwindow.h/cpp, main.cpp, tray.h/cpp

Implementation:

- LibraryConfig struct holds path, hotkey, enabled flag
- ConfigManager loads/saves libraries array
- main.cpp creates one MainWindow instance per enabled library
- Each MainWindow instance has its own StickerLibrary and ThumbnailCache
- MainWindow has constructor that accepts LibraryConfig
- getLibraryConfig() returns window's library config
- TrayIcon has Show submenu populated with library names
- Clicking tray menu item toggles corresponding window
- Each window has custom title: "Stickers Manager - [Library Name]"

## 2. Global Hotkey Management

Files: main.cpp, convertcodetostring.hpp/cpp, globalinputlistener.h/cpp

Implementation:

- GlobalInputListener created once in main.cpp
- Hotkey-to-window map built at startup
- Each window can have its own hotkey from LibraryConfig
- Hotkey press toggles corresponding window visibility
- Case-insensitive, space-insensitive, order-insensitive comparison
- No hotkey management inside MainWindow anymore

## 3. Tray Icon Show Submenu

Files: tray.h/cpp, main.cpp

Implementation:

- TrayIcon has showSubMenu (QMenu) instead of single action_showWin
- updateShowMenu() method takes QVector<LibraryConfig>
- Menu items use directory name as text, library path as data
- main.cpp connects QMenu::triggered signal to toggle windows
- Menu updates only at startup (for now)

## 4. Image Preview Dialog

Files: imagepreviewdialog.h/cpp, stickercell.h/cpp, mainwindow.h/cpp

Implementation:

- ImagePreviewDialog: Frameless window with dark background
- Shows image scaled to 80% of screen size
- Click anywhere or press Escape/Enter/Space to close
- StickerCell emits rightClicked signal on right mouse button
- MainWindow::onStickerRightClicked shows/hides preview
- Static pointer tracks current preview for toggle behavior
- ImageLoader::loadImage returns QImage, converted to QPixmap

## 5. Responsive Layout

Files: mainwindow.h/cpp

Implementation:

- resizeEvent triggers recalculateGridColumns()
- Calculates columns based on scroll area width and cell size
- Maintains minimum columns from config
- Re-displays stickers when column count changes
- Initialization still uses config values

## 6. Thumbnail Caching

Files: thumbnailcache.h/cpp, asyncthumbnailloader.h/cpp

Implementation:

- Async loading using QtConcurrent
- LRU cache with configurable size
- Signals thumbnailReady when loaded
- Both StickerCell and CategoryButton use this cache
- Pending requests tracked to handle callbacks correctly
- Each MainWindow has its own ThumbnailCache instance

## 7. Sticker Copy to Clipboard

Files: stickercell.cpp

Implementation:

- Double-click triggers copy
- Uses QMimeData with both URLs and text fallback
- Verifies copy success with QTimer
- Shows tray notification on success
