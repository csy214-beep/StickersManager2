# Project Index

C++20 / Qt 6.10.1 / Windows sticker management tool.

## Reading Order

1. [architecture.md](architecture.md) — Component hierarchy and execution flow
2. [files.md](files.md) — File-by-file source map
3. [config.md](config.md) — Config format and API
4. [features.md](features.md) — Feature internals
5. [changelog.md](changelog.md) — History

## Entry Points

- `src/main.cpp` — App entry, multi-window orchestration, hotkey wiring
- `src/mainwindow.h/cpp` — Per-library window UI
- `src/configmanager.h/cpp` — Config I/O and hot-reload
