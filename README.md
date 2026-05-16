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

StickersManager is a Windows Sticker Management Tool built with C++20 and Qt 6.10.1. It provides a user-friendly interface for managing and using sticker packs from local folders.

![img.png](GitHub/img.png)

## Features

- **Multi-Window Support**: One window per enabled sticker library
- **Global Hotkeys**: Customizable shortcuts to open/close windows
- **Local Library**: Use local folders as sticker libraries, with subdirectories as categories
- **Search Functionality**: Search stickers by file name; search categories by name
- **Easy Copy**: Double-click to copy stickers to clipboard
- **Image Preview**: Right-click to preview full-size images
- **Responsive Layout**: Adapts to window size while respecting configuration
- **First-Time Setup**: Guided library selection for new users
- **Hot-Reload**: Rescan button reloads config and picks up new libraries at runtime

## Project Structure

```txt
StickersManager/
├── CMakeLists.txt
├── resources.qrc
├── assets/
│   ├── icon.rc
│   ├── menu.qss
│   ├── st.ico
│   └── st.png
├── src/             # all source files
├── thirdparty/
│   └── stb/         # stb_image, stb_image_resize2
└── docs/.ai/        # internal dev docs
```

## Third-Party Libraries

- [Qt 6.10.1](https://www.qt.io/)
- [stb_image](https://github.com/nothings/stb)

## Config

Config file at `[EXE_DIR]/.stickersmanager/config.json`:

```json
{
    "version": 1,
    "libraries": [
        {
            "enabled": true,
            "hotkey": "Ctrl+Shift+E",
            "path": "D:/stickers/anime"
        },
        {
            "enabled": true,
            "hotkey": "Ctrl+Shift+A",
            "path": "D:/stickers/emotes"
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
        "searchDelayMs": 300
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

## License

[MIT](LICENSE)
