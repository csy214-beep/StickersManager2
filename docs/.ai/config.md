# Configuration System

## Config File Location

`[EXE_DIR]/.stickersmanager/config.json`

## Structure

### Root Level

- `version`: Schema version (currently 1)
- `libraries[]`: Array of library configurations (multi-library)
- `window`: `{position: [x,y], size: [w,h]}`
- `ui`: UI layout settings
- `behavior`: Interaction behavior
- `performance`: Cache tuning

### Library Entry (libraries[])

- `path`: File system path to sticker library
- `hotkey`: Hotkey string (e.g. `"Ctrl+Shift+E"`)
- `enabled`: Boolean — creates window + registers hotkey

### UI Settings (ui)

- `categoryButtonSize`: Pixel size of category buttons (default: 90)
- `gridCellSize`: Pixel size of sticker cells (default: 120)
- `gridColumns`: Minimum columns in grid (default: 3)

### Behavior Settings (behavior)

- `copyOnDoubleClick`: Copy on double click (default: true)
- `highlightOnClick`: Highlight on click (default: true)
- `searchDelayMs`: Search debounce in ms (default: 300)

### Performance Settings (performance)

- `thumbnailCacheSize`: Number of cached thumbnails (default: 200)

### Backward Compatibility

Old flat-format keys (`windowPosition`, `windowSize`, `libraryPath`, `shortcuts`) are still read if the new nested keys are absent.

## ConfigManager API

- `getLibraries()`: Returns `QVector<LibraryConfig>`
- `setLibraries(libs)`: Sets all libraries
- `addLibrary(lib)`: Appends one library
- `reloadFromDisk()`: Re-reads config file at runtime (used by Rescan button hot-reload)
- `saveConfig()`: Atomically writes config to disk

## Multi-Window Behavior

- One `MainWindow` per enabled library
- Each window has its own `StickerLibrary` and `ThumbnailCache`
- Windows hidden by default at startup
- Tray "Show" submenu lists all enabled libraries
- Each library can have its own hotkey (managed centrally in main.cpp)
- Rescan hot-reloads config: creates windows for new libraries, updates tray menu + hotkey map

## Configuration Safety

- **Read-only by default**: Config file is only written during first-time setup, explicit addLibrary calls, or Rescan
- **No auto-save**: Window resize/position changes are never written to config
- **User edits respected**: All manual changes to config.json are preserved (not overwritten)

## First-Time Setup

1. Program detects no valid library configured
2. Shows welcome dialog explaining library structure (subdirectories = categories)
3. Guides user to select sticker library folder
4. Saves config with `Ctrl+Shift+E` as default hotkey
