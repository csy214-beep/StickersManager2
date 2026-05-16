# StickersManager — AGENTS.md

## Quick start
```bash
cd build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug && mingw32-make
```
**Required**: Qt 6.10.1 MinGW from `D:/Qt/6.10.1/mingw_64` (hardcoded in `CMakeLists.txt:16`). Adjust path for other setups.

## Build flags
Set at top of `CMakeLists.txt`:
- `D_OR_R` = ON (Debug) / OFF (Release)
- `CONSOLE` = ON — `qDebug()` visible in console; OFF → GUI-only (`WIN32_EXECUTABLE`)

## Key architecture
- **Single instance**: `QLockFile` in `%TEMP%/<user>_Stickers Manager.lock` (main.cpp:37)
- **Config**: `[EXE_DIR]/.stickersmanager/config.json` — clean JSON with `version`, `libraries[]`, `window.{position,size}`, `ui`, `behavior`, `performance`. Backward-compat reads old flat `windowPosition`/`windowSize` keys.
- **Multi-window**: one `MainWindow` per enabled library; each has own `StickerLibrary` + `ThumbnailCache`
- **Global hotkeys**: Win32 `SetWindowsHookEx(WH_KEYBOARD_LL)` via `GlobalInputListener` (keyboard only, no mouse — removed as dead code)
- **Config hot-reload**: Rescan calls `reloadFromDisk()` → creates windows for new libraries, rebuilds hotkey map + tray menu, rescans all libraries
- **Async thumbnails**: `QtConcurrent` + `QThreadPool` via `AsyncThumbnailLoader`; LRU `QCache` in `ThumbnailCache`
- **Image loading**: `stb_image` (primary), Qt (fallback) — no libwebp support (removed)
- **Tray**: `TrayIcon` singleton with per-library "Show" submenu
- **Category search**: `QLineEdit` above left sidebar filters category buttons

## Important conventions
- **No auto-save**: config never written during `resizeEvent` or normal usage; only on first-time setup or explicit save
- **No tests / no CI / no linters**
- **Logging**: `log/log.log` (relative to exe), default level `Warning`. Toggle via `CONSOLE` build flag.
- **Packaging**: Inno Setup script in `pkg.iss` (version `ver20260501.7`)
- **Source**: all `.cpp`/`.h`/`.hpp` in `src/` are globbed — no need to register new files
- **Resources**: `resources.qrc` lists `st.png`, `menu.qss`, `st.ico` under `/` prefix (`:/assets/...`)
- **Qt auto**: `CMAKE_AUTOMOC`, `AUTORCC`, `AUTOUIC` are ON
- **Window title format**: `"Stickers Manager - <dirName>"`

## Directory layout
```
src/               # all source (flat, no subdirs)
assets/            # icons, QSS stylesheet (menu.qss), icon.rc
thirdparty/stb/    # stb_image.h, stb_image_resize2.h
docs/.ai/          # architecture/design docs (may be stale)
```

## If you need to change behavior
- Config fields are in `LibraryConfig` struct (path, hotkey, enabled)
- Window-to-hotkey mapping rebuilt by `rebuildHotkeyMapping()` at `main.cpp:19-27`
- Rescan hot-reload at `main.cpp:161-188` — the key wiring point for new libraries
- Right-click → `StickerCell` emits `rightClicked` → `ImagePreviewDialog`
- Double-click → copies sticker to clipboard via `QMimeData`
- Category search filtering at `MainWindow::onCategorySearchTextChanged` (show/hide buttons by name)
