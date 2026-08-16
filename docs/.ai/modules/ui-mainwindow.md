# MainWindow 模块

## 职责

每库一个窗口：分类面板 + 贴纸网格，滚动虚拟化渲染，搜索（贴纸/分类），复制/预览交互，缓存有效设置（m_eff）。

## 关键文件

| 文件路径 | 作用 |
|---|---|
| `src/ui/mainwindow.h` | `EffectiveSettings` 结构（:23）、成员、槽/私有方法声明 |
| `src/ui/mainwindow.cpp` | initUI、populateCategories、updateVisibleCells、搜索、事件 |
| `src/ui/stickercell.h/.cpp` | 贴纸单元格：缩略图 + 名称/大小/类型标签 + GIF 生命周期 |
| `src/ui/categorybutton.h/.cpp` | 分类按钮：名称/计数 + 可选时钟图标 |

## 数据流

```
m_config + LibraryConfig → 构造 → loadLibrary()（StickerLibrary 扫描）→ populateCategories()
显示分类 → displayStickers → relayoutGrid → updateVisibleCells（只建视口内 StickerCell）
滚动 → recalculateGridColumns → updateCellVisibility → setInViewport（GIF 装卸）
搜索 → onSearchTextChanged → delayedSearch（QTimer）→ performSearch → displayStickers
双击 → onStickerDoubleClicked → 复制 + RecentUsageStore::add
```

## 依赖关系

- 依赖：ConfigManager、StickerLibrary、ThumbnailCache、AsyncThumbnailLoader、StickerCell、CategoryButton、ImagePreviewDialog、RecentUsageStore
- 被依赖：main.cpp（createWindows）、TrayIcon（showSubMenu）

## 关键规则

- **新设置三处**：`EffectiveSettings` 字段 + `refreshEffectiveSettings()` 填充 + 热路径读 `m_eff`（updateVisibleCells）
- 可见性由 showWindow()/hide() 控制；closeEvent 与 resizeEvent 回写窗口几何
- Recent 伪分类：空隐藏、首用现场创建、右键清空（见 core-recent）
- 窗口标题由 main.cpp 设 `"Stickers Manager - <dirName>"`

## 变更记录索引

- 2025-01-14：docs/.ai Init（首次记录）
