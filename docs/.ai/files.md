# File Structure

## Root Files

- CMakeLists.txt: Build configuration
- README.md: Human-readable README
- LICENSE: License file
- resources.qrc: Qt resource file
- .gitignore: Git ignore rules

## Directory Structure

### src/

Application source code

#### Core Files

- main.cpp: Application entry point with multi-window management
- mainwindow.h/cpp: Main window UI and logic, supports per-library construction
- configmanager.h/cpp: Configuration management with multi-library support
- stickerlibrary.h/cpp: Sticker library model (per window)
- thumbnailcache.h/cpp: Thumbnail cache system (per window)
- imageloader.h/cpp: Image loading utilities
- tray.h/cpp: System tray icon with Show submenu

#### Widgets

- stickercell.h/cpp: Individual sticker widget
- categorybutton.h/cpp: Category button widget
- custommenu.h/cpp: Custom menu widget
- imagepreviewdialog.h/cpp: Image preview dialog

#### Utilities

- log.hpp: Logging system
- launcher.hpp: File/URL launcher
- checkSingleInstance.hpp: Single instance check via TCP port
- convertcodetostring.hpp/cpp: Key code to string conversion
- globalinputlistener.h/cpp: Global input listener (managed in main)
- asyncthumbnailloader.h/cpp: Async thumbnail loader

### assets/

- icon.rc: Windows icon resource
- st.ico: Application icon
- st.png: Icon image
- menu.qss: Menu stylesheet
- window.qss: Main window stylesheet
- window2.qss: Alternative stylesheet

### thirdparty/

- stb/: stb_image and stb_image_resize libraries

### docs/.ai/

AI-focused documentation
