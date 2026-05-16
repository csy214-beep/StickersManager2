# StickersManager — AGENTS.md

## Quick start
```bash
cd build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug && mingw32-make
```
**Required**: Qt 6.10.1 MinGW from `D:/Qt/6.10.1/mingw_64` (hardcoded in `CMakeLists.txt:16`). Adjust path for other setups.

## Build flags
Set at top of `CMakeLists.txt`:
- `D_OR_R` = ON (Debug) / OFF (Release) — controls `CMAKE_BUILD_TYPE`
- `CONSOLE` = ON — enables console output with `qDebug()`; OFF → GUI-only (WIN32_EXECUTABLE)

## Key architecture
- **Single instance**: `QLockFile` in `%TEMP%/<user>_Stickers Manager.lock` (main.cpp:28)
- **Config**: `[EXE_DIR]/.stickersmanager/config.json` — read-only at runtime, only written on first-time setup or corruption recovery
- **Multi-window**: one `MainWindow` per enabled library in config; each has own `StickerLibrary` + `ThumbnailCache`
- **Global hotkeys**: Win32 `SetWindowsHookEx(WH_KEYBOARD_LL)` via `GlobalInputListener`, managed centrally in `main.cpp`
- **Async thumbnails**: `QtConcurrent` + `QThreadPool` via `AsyncThumbnailLoader`; LRU `QCache` in `ThumbnailCache`
- **Image loading**: `stb_image` (primary), Qt (fallback) — in `ImageLoader`
- **Tray**: `TrayIcon` singleton with per-library "Show" submenu

## Important conventions
- **No auto-save**: config is never written during `resizeEvent` or normal usage
- **No tests / no CI / no linters** — this is a Qt Desktop app with zero test infrastructure
- **Logging**: `log/log.log` (relative to exe), default level `Warning`. Toggle via `CONSOLE` build flag.
- **Packaging**: Inno Setup script in `pkg.iss` (version `ver20260501.7`)
- **Source**: all `.cpp`/`.h`/`.hpp` in `src/` are globbed — no need to register new files in CMakeLists.txt
- **Resources**: `resources.qrc` lists assets under `/` prefix (used as `:/assets/st.png` etc.)
- **Qt auto**: `CMAKE_AUTOMOC`, `AUTORCC`, `AUTOUIC` are ON
- **Window title format**: `"Stickers Manager - <dirName>"`

## Directory layout
```
src/               # all source (flat, no subdirs)
assets/            # icons, QSS stylesheets, icon.rc
thirdparty/stb/    # stb_image.h, stb_image_resize2.h
docs/.ai/          # architecture/design docs
```

## If you need to change behavior
- Config fields are in `LibraryConfig` struct (path, hotkey, enabled)
- Window-to-hotkey mapping built in `main.cpp:139-145`
- Right-click: `StickerCell` emits `rightClicked`, `MainWindow::onStickerRightClicked` shows `ImagePreviewDialog`
- Double-click: copies sticker to clipboard via `QMimeData`
