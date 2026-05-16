# File Structure

## Root

- `CMakeLists.txt`: Build config, globs all `.cpp`/`.h`/`.hpp` from `src/`
- `resources.qrc`: Qt resource file (st.png, menu.qss, st.ico)
- `pkg.iss`: Inno Setup packaging script

## src/ — Application Source

### Core

| File | Role |
|------|------|
| `main.cpp` | Entry point; creates windows, hotkey map, tray, connects Rescan hot-reload |
| `mainwindow.h/cpp` | Main window UI; per-library instance with category panel + sticker grid |
| `configmanager.h/cpp` | JSON config I/O with hot-reload support (`reloadFromDisk()`) |
| `stickerlibrary.h/cpp` | Directory scanner; maps subdirectories → categories |
| `thumbnailcache.h/cpp` | LRU `QCache` wrapper with async loading signals |
| `imageloader.h/cpp` | Image loading: stb_image (primary), Qt (fallback) |
| `tray.h/cpp` | System tray singleton with per-library "Show" submenu |

### Widgets

| File | Role |
|------|------|
| `stickercell.h/cpp` | Individual sticker widget; click → highlight, double-click → clipboard copy |
| `categorybutton.h/cpp` | Category selector with thumbnail |
| `custommenu.h/cpp` | Round-cornered `QMenu` + `QProxyStyle` with QSS styling |
| `imagepreviewdialog.h/cpp` | Frameless full-size image preview |

### Utilities

| File | Role |
|------|------|
| `log.hpp` | Thread-safe logging to `log/log.log`; level filtering, console toggle |
| `launcher.hpp` | Open file/URL via `QDesktopServices` (async) |
| `convertcodetostring.hpp/cpp` | Virtual key code → string mapping; hotkey comparison |
| `globalinputlistener.h/cpp` | Win32 `WH_KEYBOARD_LL` global keyboard hook (keyboard only) |
| `asyncthumbnailloader.h/cpp` | `QtConcurrent` + `QThreadPool` thumbnail batch loader |

## assets/

- `icon.rc`: Windows resource script binding st.ico
- `st.ico` / `st.png`: Application icon
- `menu.qss`: QSS stylesheet for CustomMenu

## thirdparty/stb/

- `stb_image.h`, `stb_image_resize2.h`

## docs/.ai/

Internal developer/AI documentation for this project. May lag behind the code; trust `AGENTS.md` and source files as truth.
