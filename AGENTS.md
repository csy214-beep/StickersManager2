# StickersManager — AGENTS.md

## Quick start
```bash
cd build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug && mingw32-make
```
**Required**: Qt 6.10.1 MinGW from `D:/Qt/6.10.1/mingw_64` (hardcoded in `CMakeLists.txt:16`). Adjust path for other setups.
Build copies Qt DLLs + `plugins/` (platforms + imageformats) to output.

## Build flags
Set at top of `CMakeLists.txt`:
- `D_OR_R` = ON (Debug) / OFF (Release)
- `CONSOLE` = ON — console window present (`WIN32_EXECUTABLE FALSE`) so `qDebug()` visible; OFF → GUI-only

## Key architecture
- **Single instance**: `QLockFile` in `%TEMP%/<user>_Stickers Manager.lock` (main.cpp:37)
- **Config**: `[EXE_DIR]/.stickersmanager/config.json` — clean JSON with `version`, `libraries[]`, `window.{position,size}`, `ui`, `behavior` (includes `animateThumbnails`/`animatePreview`, both default `false`), `performance`. Backward-compat reads old flat `windowPosition`/`windowSize` keys.
- **Multi-window**: one `MainWindow` per enabled library; each has own `StickerLibrary` + `ThumbnailCache`
- **Global hotkeys**: Win32 `SetWindowsHookEx(WH_KEYBOARD_LL)` via `GlobalInputListener` (keyboard only)
- **Config hot-reload**: Rescan calls `reloadFromDisk()` → creates windows for new libraries, rebuilds hotkey map + tray menu, rescans all libraries (main.cpp:161-188)
- **Async thumbnails**: `QtConcurrent` + `QThreadPool` via `AsyncThumbnailLoader`; LRU `QCache` in `ThumbnailCache`
- **Animated GIF**: `QMovie` + `QBuffer` with `CacheNone` (frame-by-frame decode, no full decode). `StickerCell` only loads animation when in scroll viewport; `unloadAnimation()` releases everything (movie + buffer). `ImagePreviewDialog` async-loads via `QtConcurrent::run`. Build copies `plugins/imageformats/qgif.dll` to output. Detection: `ImageLoader::isAnimated()` uses extension + `QImageReader::imageCount()`.
- **Image loading**: `stb_image` (primary), Qt (fallback)
- **Tray**: `TrayIcon` singleton with per-library "Show" submenu; has hardcoded `Version = "1.6.0"` (tray.h:13)
- **Category search**: `QLineEdit` above left sidebar filters category buttons by name
- **No `.ui` files**: AUTOUIC is ON but unused; all UI built in code

## Important conventions
- **No auto-save**: config never written during `resizeEvent` or normal usage; only on first-time setup or explicit save
- **No tests / no CI / no linters**
- **Logging**: `log/log.log` (relative to exe), default level `Warning`. Toggle via `CONSOLE` build flag.
- **Packaging**: Inno Setup script in `pkg.iss`
- **Source**: all `.cpp`/`.h`/`.hpp` in `src/` are globbed — no need to register new files
- **Resources**: `resources.qrc` lists `st.png`, `menu.qss`, `st.ico` under `/` prefix (`:/assets/...`)
- **Qt auto**: `CMAKE_AUTOMOC`, `AUTORCC`, `AUTOUIC` are ON
- **Window title format**: `"Stickers Manager - <dirName>"`
- **Style**: always sets Fusion (`main.cpp:48`)
- **Debug mode**: `#define DEBUG_MODE false` at top of `main.cpp`; when true, disables `qInstallMessageHandler`
- **Header-only utils**: `log.hpp` and `launcher.hpp` use `static` functions (one copy per TU). `launch()` in `launcher.hpp` opens paths/URLs via `QtConcurrent::run` + `QDesktopServices` — used for tray's settings/open-repo actions.

## Directory layout
```
src/               # all source (flat, no subdirs)
assets/            # icons, QSS stylesheet (menu.qss), icon.rc
thirdparty/stb/    # stb_image.h, stb_image_resize2.h
GitHub/            # README screenshots (img.png)
docs/.ai/          # architecture/design docs (may be stale)
```

## Key entry points for changes
- `LibraryConfig` struct (configmanager.h:16-25): path, hotkey, enabled fields
- `rebuildHotkeyMapping()` at main.cpp:19 — maps hotkey strings to MainWindow pointers
- Rescan hot-reload at main.cpp:161-188 — wiring point for new library lifecycle
- `TrayIcon::Version` (tray.h:13) — manual version string, only place updated on release
