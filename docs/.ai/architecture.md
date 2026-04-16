# Architecture

## Project Type

Qt Widgets Application
Static Export (Pure Desktop App)

## Tech Stack

- Language: C++20
- Framework: Qt 6.10.1 (MinGW)
- Build System: CMake
- Image Library: stb_image

## Core Components

### Main Flow

1. main.cpp initializes app
2. ConfigManager loads/saves config
3. main.cpp creates MainWindow instances for each enabled library
4. Each MainWindow creates its own UI and loads its library
5. StickerLibrary scans directories per window
6. ThumbnailCache manages image caching per window
7. Global hotkey management is centralized in main.cpp
8. TrayIcon shows submenu for library selection

### Component Hierarchy

Application
├── TrayIcon (Singleton)
│ └── Show submenu with library actions
├── ConfigManager (Singleton in main)
├── GlobalInputListener (Singleton in main)
└── Multiple MainWindow instances (one per library)
├── ConfigManager reference
├── StickerLibrary (Library-specific)
├── ThumbnailCache (Window-specific)
├── CategoryPanel (Left Sidebar)
│ └── CategoryButton widgets
└── StickerPanel (Right Panel)
├── Search bar
└── StickerCell widgets in grid

## Key Classes

- ConfigManager: JSON config file I/O with multi-library support
  - **Important**: Read-only by default, only modifies config on first-run setup or corruption
  - No auto-save on window resize
- LibraryConfig: Struct for per-library settings (path, hotkey, enabled)
- StickerLibrary: Directory scanning and search per library
- ThumbnailCache: Async thumbnail loading per window
- StickerCell: Individual sticker widget
  - Left/right click both set selection highlight
  - Right double-click does NOT trigger copy or close
- CategoryButton: Category selector widget
- ImagePreviewDialog: Full-size image preview
- GlobalInputListener: Global keyboard hook (managed in main)
- TrayIcon: System tray with Show submenu

## Configuration Philosophy

- **User config first**: Manual edits to config.json are always respected
- **Minimal writes**: Config file only written when absolutely necessary
- **First-time setup**: Guides user through initial library selection
- **Safety first**: Only overwrites config if it would cause program crash
