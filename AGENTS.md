# StickersManager — AGENTS.md

## Build & config
```bash
cd build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug && cmake --build . -- -j8
```
- Qt 6.10.1 MinGW hardcoded at `CMakeLists.txt:16` (`D:/Qt/6.10.1/mingw_64`)
- `CMakeLists.txt` links `Qt6::Core Gui Widgets Concurrent Network`; new source files need re-run `cmake ..` to be picked up (`FILE(GLOB_RECURSE ./src/)` for `.cpp/.h/.hpp`)
- `CONSOLE` ON shows console (`WIN32_EXECUTABLE FALSE`), `qDebug()` visible — **currently OFF** (release). C++20, `UNICODE`/`_UNICODE` defined
- `windeployqt` auto-deploy runs POST_BUILD on every build (`CMakeLists.txt:103-113`); C++20, `UNICODE`/`_UNICODE` defined

## Architecture
- **Single instance**: `QLockFile` at `%TEMP%/<user>_Stickers Manager.lock` (`main.cpp:44`)
- **Config** (`[EXE_DIR]/.stickersmanager/`, relative to exe):
  - `config.json`: `{"default":{ui,behavior,window,performance},"libraries":[{id,path,hotkey,enabled,settings:{}}]}` — no version field; `id` is the **order marker** (0..n-1, contiguous): `getLibraries()` sorts by id (`configmanager.cpp:145-167`), missing ids (legacy) get array position, drag-reorder in the Libraries tab renumbers ids; `enabled` = **hotkey switch only** ("Enable Hotkey" checkbox) — does NOT affect window/tray/menu/double-click target/storage stats, only global-hotkey registration and hotkey-conflict computation
  - `settings.json`: `doubleClickTarget` (default `"settings"`; `"first-library"`/library id string; legacy dirName/path values still matched as fallback, `main.cpp:242-259`), `searchDelayMs` (300), `thumbnailCacheSize` (200), `checkForUpdatesOnStartup` (true), `startWithWindows` (false)
  - `recent_<md5(libraryPath)>.json`: per-library **recently-used** list `{"entries":[{path,time}]}` (storage capped 100, newest first, stale files pruned on load); written by `RecentUsageStore` (`src/core/recentusage.cpp`) on double-click copy; shown as the first "Recent" pseudo-category in `MainWindow::populateCategories()` — **hidden entirely when empty**, created live on first use (`onStickerDoubleClicked` → `refreshRecentButton()` returns false → repopulate); right-click on it shows a "Clear Recent Records" menu (`onCategoryContextMenuRequested`, `CustomMenu` style, `RecentUsageStore::clear()` deletes the file, then repopulate hides it); its preview icon is a clock (`setShowClock`), drawn from `:/assets/Clock - 24x24.png` tinted via `SourceIn` (`clockicon.cpp` `makeClockIcon()`); display cap = `recentLimit` (`default.ui`, default 100, per-library override) applied in `RecentUsageStore::paths()`; count label refreshed live via `MainWindow::refreshRecentButton()`
- **No auto-save** — config written on explicit `saveConfig()` / `saveSettings()` only (Settings "Save & Reload", `settingsdialog.cpp:66-67`), plus the fallback folder-picker save in `mainwindow.cpp:150` when a window has an empty path
- **First-launch** — if no library with existing path, modal `SettingsDialog tmpDlg.exec()` (`main.cpp:197-203`) blocks before any window/tray
- **Multi-window**: one `MainWindow` per library with non-empty path (regardless of hotkey switch); title `"Stickers Manager - <dirName>"` (`main.cpp:72`)
- **Scanning** (`stickerlibrary.cpp`): **immediate subdirectories** only are categories (non-recursive); `isPreviewFile()` (name starts with `.preview` or contains `.preview.`) files excluded from scan/search/stats (`fsutil.hpp:26`); search matches against a prebuilt lowercase-filename index
- **Global hotkeys**: Win32 `SetWindowsHookEx(WH_KEYBOARD_LL)` via `GlobalInputListener`; key↔string mapping + `ShortcutCompare::compareShortcutKeys` in `convertcodetostring.hpp`; listener auto-starts/stops when hotkey map empties (`main.cpp:116-128`)
- **Two runtime reload paths** (don't confuse them):
  - `fullReload` (`main.cpp:130`) — tray **Rescan**: reloads config, removes stale windows, creates new ones, rescans all libs via `reloadLibrary()`
  - `applyChanges` (`main.cpp:142`) — settings dialog `applied` signal: updates per-window `LibraryConfig` + reapplies settings, **no rescan**
- **Image loading**: `stb_image` (primary), `QImageReader` (fallback) in `imageloader.cpp`
- **Async thumbnails**: `QtConcurrent` + `QThreadPool` via `AsyncThumbnailLoader`; LRU `QCache` in `ThumbnailCache`
- **Startup update check**: `UpdateChecker` hits GitHub API (`apiReleasesUrl()`), gated by `checkForUpdatesOnStartup`

## Override system
- `getEffectiveXxx(lib)` — checks `lib.settings[cat][key]`, falls back to `default[cat][key]`, then hardcoded default
- 0 = "not overridden" for numeric; "General"/"On"/"Off" for booleans
- `getDefaultXxx()` reads from `default{}` (set by General tab)
- Tag toggles (4 keys in `default.behavior`, default true): `showStickerName`, `showStickerSize`, `showCategoryName`, `showCategoryCount` (General tab + per-library combos)
- `recentLimit` (`default.ui`, default 100, per-library numeric override, 0 = not overridden): recent display cap

## Animation (GIF)
- Detected by extension `.gif` or `QImageReader::imageCount() > 1`
- `QMovie + QBuffer + CacheNone` — buffer must outlive movie (set as parent)
- `StickerCell::setInViewport()` — calls `loadAnimation()` on scroll-in, `unloadAnimation()` on scroll-out (zero memory for off-screen)
- `ImagePreviewDialog`: same async pattern; `WA_DeleteOnClose` frees dialog+movie on close

## SettingsDialog
- 4-tab `QTabWidget`: General / Libraries / Base / About (`settingsdialog.cpp:33-36`)
- "Save & Reload" (`onSave`, `settingsdialog.cpp:60`): applies all pages, `saveConfig()` + `saveSettings()`, emits `applied` → `applyChanges`; keeps dialog open
- Implemented as `QPointer<SettingsDialog>` singleton with `WA_DeleteOnClose`; tray double-click toggles open/close (`main.cpp:171-186`)
- Libraries tab: drag-reorder renumbers ids; hotkey set by key capture (`hotkeycapture.cpp`); Base tab refreshes the double-click-target combo on tab show (`settingsdialog.cpp:40-45`)

## File type tag
- `QLabel` overlay with uppercase extension, positioned absolutely at `(7, cellSize - tagHeight - 7)` (`stickercell.cpp:54`). Not in layout.
- Hidden during placeholder; shown only after real thumbnail; controlled by `getEffectiveShowFileTypeTag()`
- `StickerCell` also overlays file name without extension (top-left `(7,7)`, elided) and file size (bottom-right, via `formatBytes`) — shown immediately, gated by `getEffectiveShowStickerName()` / `getEffectiveShowStickerSize()`
- `CategoryButton` overlays name (bottom-left, elided) and count (bottom-right), gated by `getEffectiveShowCategoryName()` / `getEffectiveShowCategoryCount()`; optional clock as the button **preview icon** (`clockicon.cpp` `makeClockIcon()`, tinted `:/assets/Clock - 24x24.png`) via `setShowClock()` — used only for the Recent category

## Directory layout
```
src/
├── main.cpp
├── appinfo.h          # inline namespace AppInfo — version, author, license, repo/issue URLs
├── core/              # config, library, image loading, cache, input, keycode map, update checker, recent usage
├── ui/                # mainwindow, tray, dialogs, cells, category buttons, custommenu, hotkey capture, settings pages
└── utils/             # log.hpp, launcher.hpp, fsutil.hpp (all header-only `static`)
assets/                # icons, menu.qss, icon.rc
scripts/clean_release.py   # deletes build artifacts from release/ (edit RUN/DIRS at top)
thirdparty/stb/        # stb_image.h, stb_image_resize2.h
```

## Conventions
- **All source globbed**: new files need re-run `cmake ..` to be picked up
- **No `.ui` files**: `AUTOUIC ON` but all UI in C++ code; `CMAKE_AUTOMOC` + `AUTORCC` also ON
- **Style**: Fusion (`main.cpp:53-55`)
- **Logging**: `log.hpp` writes to `log/log.log` relative to working dir, truncated on each start; `DEBUG_MODE` in `main.cpp:24` — `false` installs `messageHandler`, `true` shows `qDebug()` in console
- **Version**: `appinfo.h:7` — `AppInfo::version()` returns `"2.6.0"`, manual bump only on release
- **About page**: version comparison strips `v` prefix, parses `major.minor.patch` numerically (`UpdateChecker::compareVersions`)
- **Header-only utils**: `static` functions in `log.hpp` / `launcher.hpp` / `fsutil.hpp` (one copy per TU). `launch()` opens paths/URLs via `QtConcurrent::run` + `QDesktopServices`; `notifyTray()` marshals tray messages from worker threads to the GUI thread (`QueuedConnection`)
- **No tests / CI / linters**; packaging via `pkg.iss` (Inno Setup); release artifacts (`StickersManager_<ver>.exe/.7z`) at repo root are **gitignored** (`*.exe`/`*.7z`) — local only, never committed