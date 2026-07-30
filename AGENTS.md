# StickersManager — AGENTS.md

## Build & config
```bash
cd build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug && cmake --build . -- -j8
```
- Qt 6.10.1 MinGW hardcoded at `CMakeLists.txt:16` (`D:/Qt/6.10.1/mingw_64`)
- `CMakeLists.txt` links `Qt6::Core Gui Widgets Concurrent Network`; new source files need `cmake ..` to be picked up (`GLOB_RECURSE`)
- `D_OR_R` ON=Debug, OFF=Release (only affects Qt debug libs)
- `CONSOLE` ON — shows console (`WIN32_EXECUTABLE FALSE`), `qDebug()` visible

## Architecture
- **Single instance**: `QLockFile` in `%TEMP%/<user>_Stickers Manager.lock` (`main.cpp:42`)
- **Config** (`[EXE_DIR]/.stickersmanager/`):
  - `config.json` v2: `{"default":{ui,behavior,window},"libraries":[{path,hotkey,enabled,settings:{}}]}`
  - `settings.json`: `doubleClickTarget` (`"first-library"`/`"settings"`/dirName), `searchDelayMs`, `thumbnailCacheSize`
- **No auto-save** — config written on explicit `saveConfig()` / `saveSettings()` only
- **First-launch** — if no enabled library with existing path, modal `SettingsDialog` blocks before any window/tray
- **Multi-window**: one `MainWindow` per enabled library; title set to `"Stickers Manager - <dirName>"` in `main.cpp:70`
- **Global hotkeys**: Win32 `SetWindowsHookEx(WH_KEYBOARD_LL)` via `GlobalInputListener`
- **Hot-reload**: `fullReload` lambda (`main.cpp:95-104`) — reloads config, removes stale windows, creates new ones, rescans all libs
- **Image loading**: `stb_image` (primary), `QImageReader` (fallback) in `imageloader.cpp`
- **Async thumbnails**: `QtConcurrent` + `QThreadPool` via `AsyncThumbnailLoader`; LRU `QCache` in `ThumbnailCache`

## Override system
- `getEffectiveXxx(lib)` — checks `lib.settings[cat][key]`, falls back to `default[cat][key]`, then hardcoded default
- 0 = "not overridden" for numeric; "General"/"On"/"Off" for booleans
- `getDefaultXxx()` reads from `default{}` (set by General tab)

## Animation (GIF)
- Detected by extension `.gif` or `QImageReader::imageCount() > 1`
- `QMovie + QBuffer + CacheNone` — buffer must outlive movie (set as parent)
- `StickerCell::setInViewport()` — calls `loadAnimation()` on scroll-in, `unloadAnimation()` on scroll-out (zero memory for off-screen)
- `ImagePreviewDialog`: same async pattern; `WA_DeleteOnClose` frees dialog+movie on close

## SettingsDialog
- 4-tab `QTabWidget`: General / Libraries / Base / About
- "Save & Reload" calls `saveConfig()` + `saveSettings()`, triggers `fullReload`
- Implemented as `QPointer<SettingsDialog>` singleton with `WA_DeleteOnClose`; double‑click tray toggles open/close

## File type tag
- `QLabel` overlay with uppercase extension, positioned absolutely at `(7, cellSize - tagHeight - 7)`. Not in layout.
- Hidden during placeholder; shown only after real thumbnail; controlled by `getEffectiveShowFileTypeTag()`

## Directory layout
```
src/
├── main.cpp
├── appinfo.h          # inline namespace AppInfo — version, author, license, repo/issue URLs
├── core/              # config, library, image loading, cache, input
├── ui/                # mainwindow, tray, dialogs, cells, category buttons, settings pages
└── utils/             # log.hpp (static), launcher.hpp (static)
assets/                # icons, menu.qss, icon.rc
thirdparty/stb/        # stb_image.h, stb_image_resize2.h
```

## Conventions
- **All source globbed**: `FILE(GLOB_RECURSE *.cpp *.h *.hpp ./src/)` — new files need re-run `cmake ..` to be picked up
- **No `.ui` files**: `AUTOUIC ON` but all UI in C++ code; `CMAKE_AUTOMOC` + `AUTORCC` also ON
- **Style**: Fusion (`main.cpp:52-53`)
- **DEBUG_MODE**: `#define DEBUG_MODE false` in `main.cpp:22`; set to `true` to see `qDebug()` in console
- **Version**: `appinfo.h:7` — `AppInfo::version()` returns `"2.0.1"`, manual bump only on release
- **About page**: version comparison strips `v` prefix, parses `major.minor.patch` numerically; update check hits GitHub API (`apiReleasesUrl()`)
- **Header-only utils**: `static` functions in `log.hpp` and `launcher.hpp` (one copy per TU). `launch()` opens paths/URLs via `QtConcurrent::run` + `QDesktopServices`.
- **No tests / CI / linters**; packaging via `pkg.iss` (Inno Setup)
