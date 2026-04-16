# Configuration System

## Config File Location

Path: [EXE_DIR]/.stickersmanager/config.json

## Config Structure

### Root Level

- libraryPath: (Legacy) Single library path
- port: TCP port for single instance check (default: 8868)
- windowPosition: [x, y] array
- windowSize: [width, height] array
- shortcuts: (Legacy) {useHotkey, hotkey}
- ui: UI settings
- behavior: Behavior settings
- performance: Performance settings
- libraries: Array of library configurations (Multi-library support)

### Library Config Object (libraries array)

Each object has:

- path: File system path to sticker library
- hotkey: Hotkey string (e.g., "Ctrl+Shift+E")
- enabled: Boolean (true to create window and listen for hotkey)

### UI Settings (ui object)

- categoryButtonSize: Pixel size of category buttons (default: 90)
- gridCellSize: Pixel size of sticker cells (default: 120)
- gridColumns: Minimum columns in grid (default: 3)

### Behavior Settings (behavior object)

- copyOnDoubleClick: Copy on double click (default: true)
- highlightOnClick: Highlight on click (default: true)
- searchDelayMs: Search debounce in ms (default: 300)

### Performance Settings (performance object)

- thumbnailCacheSize: Number of cached thumbnails (default: 200)
- lazyLoadEnabled: Lazy loading (default: true)

## Migration

Legacy single-library config auto-migrates to multi-library format on first load.

## ConfigManager API

- getLibraries(): Returns QVector<LibraryConfig>
- setLibraries(libs): Sets all libraries
- addLibrary(lib): Adds one library
- removeLibrary(index): Removes library at index
- Legacy getters still work for backward compatibility

## Multi-Window Behavior

- One MainWindow created per enabled library
- Each window has its own StickerLibrary and ThumbnailCache
- Windows are hidden by default at startup
- Tray Show submenu lists all enabled libraries
- Each library can have its own hotkey
- Hotkeys are managed centrally in main.cpp

## Configuration Safety

- **Read-only by default**: Program will not modify config file unless necessary
- **Auto-save removed**: No longer saves window size/position on every resize
- **First-time setup only**: Config is only created/modified during initial setup
- **Corruption recovery**: Will only overwrite config if it would cause program crash
- **User config respected**: All manual edits to config.json are preserved

## First-Time Setup

- Program detects if no valid sticker library is configured
- Shows welcome dialog explaining how to organize sticker library
- Guides user to select sticker library folder via file dialog
- Automatically saves config with selected library path
- Sets Ctrl+Shift+E as default hotkey for first library
