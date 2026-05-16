# Architecture

## Stack

- Language: C++20
- Framework: Qt 6.10.1 (MinGW)
- Build: CMake
- Image loading: stb_image (primary), Qt (fallback)
- Platform: Windows only (Win32 hooks, QLockFile)

## Main Flow

1. `main.cpp` initializes app, single-instance lock (`QLockFile`), Fusion style
2. `ConfigManager` loads JSON from `[EXE_DIR]/.stickersmanager/config.json`
3. If no libraries configured, first-time setup wizard runs
4. `main.cpp` creates one `MainWindow` per enabled library
5. Each `MainWindow` creates category sidebar + sticker grid, loads thumbnails
6. `GlobalInputListener` installs keyboard hook, toggles window on hotkey match
7. `TrayIcon` shows per-library "Show" submenu to toggle window visibility
8. Rescan button invokes `reloadFromDisk()` → creates windows for new libraries, rebuilds hotkey map + tray menu, rescans all libraries

## Component Hierarchy

```
Application
├── TrayIcon (singleton)
│   └── Show submenu with per-library actions
├── ConfigManager (local in main)
├── GlobalInputListener (local in main, Win32 keyboard hook)
└── MainWindow instances (one per enabled library)
    ├── CategoryPanel (left sidebar)
    │   ├── QLineEdit (category search)
    │   └── CategoryButton widgets (scrollable)
    └── StickerPanel (right)
        ├── QLineEdit (sticker file name search)
        └── StickerCell widgets in QGridLayout
```

## Key Classes

- **ConfigManager**: JSON I/O; `reloadFromDisk()` for hot-reload; read-only by default
- **LibraryConfig**: Struct with path, hotkey, enabled
- **StickerLibrary**: Scans library directory; subdirectories → categories; file name search
- **ThumbnailCache**: Per-window LRU `QCache`; signals `thumbnailReady` on async load
- **AsyncThumbnailLoader**: `QtConcurrent` + `QThreadPool` batch loader
- **StickerCell**: Click → highlight, double-click → clipboard copy (with verification)
- **CategoryButton**: Shows category thumbnail, filterable by name search
- **ImagePreviewDialog**: Frameless, dark background, 80% screen scaling
- **GlobalInputListener**: Win32 `WH_KEYBOARD_LL` hook; emits `keyReleased` with modifiers
- **TrayIcon**: Singleton with Settings/Pictures/Rescan/GitHub/Exit actions

## Configuration Philosophy

- **User config first**: Manual edits to config.json are always respected
- **Minimal writes**: Only written on first-time setup, addLibrary, or explicit saveConfig
- **No auto-save**: resizeEvent and normal usage never write to disk
- **Hot-reload**: Rescan button re-reads config and adapts at runtime (new windows, hotkeys, tray)
