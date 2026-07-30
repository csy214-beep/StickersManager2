# Changelog — v1.8.0 → v2.0.1

### Config system (breaking)
- Config v2: flat structure → `{"default":{...},"libraries":[{...,"settings":{}}]}`
- `settings.json` split from `config.json`: stores `doubleClickTarget`, `searchDelayMs`, `thumbnailCacheSize`
- Per-library override system: `getEffectiveXxx(lib)` checks `lib.settings` → `default{}` → hardcoded fallback
- `QSpinBox::setSpecialValueText("General")` (0 = not overridden), boolean combo "General"/"On"/"Off"
- Atomic file write (`.tmp` + rename) for both config files (was direct write)

### Settings dialog (new)
- 4-tab `QTabWidget`: **General** (default values), **Libraries** (override per library), **Base** (app-level), **About**
- Singleton via `QPointer<SettingsDialog>` + `WA_DeleteOnClose`; double‑click tray toggles open/close
- "Save & Reload" calls `saveConfig()` + `saveSettings()` then `fullReload`

### About page (new)
- Displays name, version, author, license, repo/issue links (from `appinfo.h`)
- **Storage**: app directory size + all library files total size (separate)
- **Paths**: executable path (opens folder) + config folder path (opens folder) — clickable
- **Update check**: fetches `api.github.com/repos/.../releases/latest`, strips `v` prefix, compares `major.minor.patch`
- Disk usage computed via `QDirIterator` (recursive)

### Override system (new: 10 `getEffective*` methods)
- `window`: size, position, alwaysOnTop
- `ui`: categoryButtonSize, gridCellSize, gridColumns
- `behavior`: copyOnDoubleClick, highlightOnClick, animateThumbnails, animatePreview, showFileTypeTag

### Application info (new: `src/appinfo.h`)
- Inline namespace `AppInfo` — single source for `version()`, `author()`, `license()`, `repoUrl()`, `issuesUrl()`, `apiReleasesUrl()`
- `"2.0.1"`, `"igugyj"`, `"MIT"`

### First-launch flow (changed)
- Before: `QFileDialog::getExistingDirectory` → save to config
- After: modal `SettingsDialog` (4-tab) blocks before any window/tray

### Tray icon (changed)
- Removed `TrayIcon::Version` (moved to `appinfo.h`)
- Tooltip now `AppInfo::name() + " " + AppInfo::version()`
- Removed `action_openRepo` (was `"Open App Dir"` conditionally); now always shows "Open App Dir"
- Removed GitHub link from tray menu

### Double-click tray behavior (changed)
- Before: always toggles first library window
- After: checks `settings.json:doubleClickTarget` — `"settings"` toggles SettingsDialog, `"first-library"`/dirName toggles window

### Dark theme compatibility (changed)
- `StickerCell` background: hardcoded `#f5f5f5` → `QPalette::Base`
- `StickerCell` highlight: hardcoded → `palette().color(QPalette::Highlight)`
- `CategoryButton` QSS: injected via palette colors instead of `#ffffff` / `#e8f4f8` / `#409eff`
- All plain `QLabel` links: `color: palette(highlight)`

### Window always‑on‑top (changed)
- `WindowStaysOnTopHint` reapplied in `showWindow()` each time (was set once in constructor)
- `showWindow()` reapplies geometry (position + size) every show
- `getEffectiveAlwaysOnTop()` with per-library override

### Highlight & copy (changed)
- `StickerCell::setHighlightEnabled(bool)` / `setCopyOnDoubleClick(bool)` — driven by config
- Highlight on click and copy on double-click are per-library configurable

### Search & cache (changed)
- `searchDelayMs` moved from `config.json` to `settings.json`; read via `m_config->getSearchDelayMs()`
- `thumbnailCacheSize` moved from `config.json` to `settings.json`; per-library via `getEffectiveThumbnailCacheSize()`

### File structure (restructured)
- Source files moved into subdirectories: `src/core/`, `src/ui/`, `src/utils/`
- `CMakeLists.txt`: `GLOB_RECURSE`, linked `Qt6::Network`, `include_directories` for subdirs
- All dialog `QFileDialog::getExistingDirectory` calls: `QFileDialog::DontUseNativeDialog`
- Removed `docs/.ai/`, `GitHub/img.gif`, stale `thirdparty/QtTheme/`

### Code quality
- `reloadLibrary()` now calls `recalculateGridColumns()`
- `populateCategories()` updates category panel width when button size changes
- `onStickerClicked()` clears highlight on other cells
- Version comparison in update check strips `v` prefix, parses numeric (major.minor.patch)
- `QFile`: atomic write pattern for both config files
- `settings.json`: no auto-save (only on explicit `saveSettings()`)
