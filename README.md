<h1 align="center">
StickersManager
</h1>
<p align="center">
    <img src="https://img.shields.io/badge/language-C%2B%2B20-blue?logo=c%2B%2B" alt="Language">
    <img src="https://img.shields.io/badge/framework-Qt%206.10.1-brightgreen?logo=qt" alt="Qt">
    <img src="https://img.shields.io/badge/platform-Windows-blue?logo=windows" alt="Windows">
    <img src="https://img.shields.io/badge/license-MIT-lightgrey" alt="License">
</p>

## Overview

StickersManager is a Windows sticker management tool built with C++20 and Qt 6.10.1. It manages sticker packs from local folders.

![img.png](GitHub/img.png)

## Features

- **Multi-window** — one window per library
- **Global hotkeys** — customizable shortcuts per library
- **Local folders** — subdirectories become categories
- **Search** — search stickers by filename, categories by name
- **Copy** — double-click to copy to clipboard
- **Preview** — right-click for full-size view
- **Hot-reload** — rescan picks up config/library changes at runtime
- **Animated GIF** — optional, controlled by config (default off)
- **First-time setup** — guided library selection

## Config

File: `[EXE_DIR]/.stickersmanager/config.json`

```json
{
    "version": 1,
    "libraries": [
        {
            "enabled": true,
            "hotkey": "Ctrl+Shift+E",
            "path": "D:/stickers/anime"
        }
    ],
    "ui": {
        "categoryButtonSize": 90,
        "gridCellSize": 120,
        "gridColumns": 3
    },
    "behavior": {
        "copyOnDoubleClick": true,
        "highlightOnClick": true,
        "searchDelayMs": 300,
        "animateThumbnails": false,
        "animatePreview": false
    },
    "performance": {
        "thumbnailCacheSize": 200
    },
    "window": {
        "position": [900, 50],
        "size": [540, 430]
    }
}
```

| Key | Description |
|---|---|
| `libraries[].path` | Path to sticker library folder |
| `libraries[].hotkey` | Global hotkey, e.g. `Ctrl+Shift+E` |
| `libraries[].enabled` | Show/hide this library |
| `ui.categoryButtonSize` | Category thumbnail size in px |
| `ui.gridCellSize` | Sticker cell size in px |
| `ui.gridColumns` | Minimum column count |
| `behavior.copyOnDoubleClick` | Copy sticker file on double-click |
| `behavior.highlightOnClick` | Highlight selected cell |
| `behavior.searchDelayMs` | Debounce delay for search input |
| `behavior.animateThumbnails` | Play GIF animation in grid cells |
| `behavior.animatePreview` | Play GIF animation in preview dialog |
| `performance.thumbnailCacheSize` | Max cached thumbnails |
| `window.position` | Window position `[x, y]` |
| `window.size` | Window size `[width, height]` |

## Project Structure

```txt
StickersManager/
├── CMakeLists.txt
├── resources.qrc
├── assets/            # icons, stylesheet, icon.rc
├── src/               # all source files
├── thirdparty/
│   └── stb/           # stb_image, stb_image_resize2
└── docs/.ai/          # internal dev docs
```

## Third-Party Libraries

- [Qt 6.10.1](https://www.qt.io/)
- [stb_image](https://github.com/nothings/stb)

## License

[MIT](LICENSE)
