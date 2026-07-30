# StickersManager — AGENTS.md

## Build
```bash
cd build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug && mingw32-make
```
Qt 6.10.1 MinGW hardcoded at `CMakeLists.txt:16` (`D:/Qt/6.10.1/mingw_64`).

Build flags (top of `CMakeLists.txt`):
- `D_OR_R` — ON=Debug, OFF=Release (unused in code, only affects Qt debug libs)
- `CONSOLE` — ON shows console window (`WIN32_EXECUTABLE FALSE`), `qDebug()` visible; OFF=headless GUI

## Architecture
- **Single instance**: `QLockFile` in `%TEMP%/<user>_Stickers Manager.lock` (main.cpp:37)
- **Config location**: `[EXE_DIR]/.stickersmanager/config.json`; backward-compat reads old flat `windowPosition`/`windowSize` keys
- **No auto-save**: config never written on resize/move/close — only on first-time setup (`config.saveConfig()`) or explicit calls
- **Multi-window**: one `MainWindow` per enabled library; title set twice — constructor sticks version string, then `main.cpp:94` replaces it with `"Stickers Manager - <dirName>"`
- **Global hotkeys**: Win32 `SetWindowsHookEx(WH_KEYBOARD_LL)` via `GlobalInputListener` (main.cpp:129-158)
- **Hot-reload**: rescan at `main.cpp:161-188` — `reloadFromDisk()` → new windows for new libraries → rebuild hotkey map + tray menu → rescan all libs
- **Image loading**: `stb_image` (primary), Qt `QImageReader` (fallback) in `imageloader.cpp`
- **Async thumbnails**: `QtConcurrent` + `QThreadPool` via `AsyncThumbnailLoader`; LRU `QCache` in `ThumbnailCache`

## Animation (GIF)
- **Config**: `behavior.animateThumbnails` (grid, default `false`), `behavior.animatePreview` (dialog, default `false`), `behavior.showFileTypeTag` (grid cell overlay, default `true`)
- **Detection**: `ImageLoader::isAnimated()` — checks `.gif` extension first, then `QImageReader::imageCount() > 1`
- **QMovie + QBuffer + CacheNone**: file read on `QtConcurrent` thread → `QByteArray` → `QBuffer` → `QMovie(buffer)`; `CacheNone` avoids full decode (each frame decoded on demand). Buffer must outlive movie (set as parent).
- **Viewport lifecycle**: `StickerCell::setInViewport()` calls `loadAnimation()` on scroll-in, `unloadAnimation()` on scroll-out (stops movie, deletes buffer+watcher). Zero memory for off-screen cells.
- **ImagePreviewDialog**: similar async pattern; has `WA_DeleteOnClose` so dialog+movie freed on close.

## File type tag
- **QLabel overlay** with extension (uppercase), positioned absolutely: `move(7, cellSize - tagHeight - 7)` in constructor. Not in layout — does not affect image centering.
- **Visibility**: tag hidden during placeholder, shown only after real thumbnail set; `setShowTag()` controls via config.

## Directory layout
```
src/
├── main.cpp
├── core/           # config, library, image loading, cache, input
├── ui/             # all UI widgets (mainwindow, tray, dialogs, cells)
└── utils/          # log.hpp, launcher.hpp
assets/            # icons, QSS stylesheet (menu.qss), icon.rc
thirdparty/stb/    # stb_image.h, stb_image_resize2.h
GitHub/            # README screenshots
docs/.ai/          # architecture/design docs (may be stale)
```

## Conventions
- **All source globbed recursively**: `FILE(GLOB_RECURSE *.cpp *.h *.hpp ./src/)` — put new files in the right subdirectory, no need to register in CMake
- **No `.ui` files**: `AUTOUIC ON` but unused; all UI in C++ code
- **Qt auto**: `CMAKE_AUTOMOC`, `AUTORCC`, `AUTOUIC` are ON
- **Style**: Fusion set at `main.cpp:48` via `QStyleFactory`
- **DEBUG_MODE**: `#define DEBUG_MODE false` at top of `main.cpp`; when `true`, disables `qInstallMessageHandler` (log visible in console)
- **Version string**: `TrayIcon::Version` (`tray.h:13`) — currently `"1.8.0"`, manual bump only on release
- **Header-only utils**: `log.hpp` and `launcher.hpp` use `static` functions (one copy per TU). `launch()` in `launcher.hpp` opens paths/URLs via `QtConcurrent::run` + `QDesktopServices`.
- **No tests / CI / linters** — no test framework installed; packaging via `pkg.iss` (Inno Setup)
