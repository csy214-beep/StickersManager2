# 图像与缩略图模块

## 职责

解码图像（主 stb_image、兜底 QImageReader），异步生成缩略图（QtConcurrent + QThreadPool），LRU 缓存，GIF 动画生命周期管理。

## 关键文件

| 文件路径 | 作用 |
|---|---|
| `src/core/imageloader.h/.cpp` | 解码：stb_image 主、QImageReader 兜底 |
| `src/core/asyncthumbnailloader.h/.cpp` | AsyncThumbnailLoader：QtConcurrent 异步缩略图 |
| `src/core/thumbnailcache.h/.cpp` | ThumbnailCache：LRU QCache（默认 200） |
| `src/ui/stickercell.cpp` | `setInViewport()`：滚入 loadAnimation / 滚出 unloadAnimation |
| `src/ui/imagepreviewdialog.cpp` | 全尺寸预览，WA_DeleteOnClose 释放 dialog+movie |

## 数据流

```
文件 → AsyncThumbnailLoader（worker 线程解码+缩放）→ ThumbnailCache（LRU）→ onThumbnailLoaded → StickerCell::setPixmap
GIF：滚入 → loadAnimation（QMovie+QBuffer+CacheNone，buffer 以 movie 为 parent）→ 滚出 → unloadAnimation（零内存）
预览：右键 → ImagePreviewDialog（async 同模式）
```

## 依赖关系

- 依赖：QtConcurrent、QThreadPool、stb（thirdparty/stb）
- 被依赖：MainWindow、ImagePreviewDialog

## 关键规则

- GIF 判定：扩展名 `.gif` 或 `QImageReader::imageCount() > 1`
- QBuffer 必须 outlive movie：设为 movie parent
- 动画开关：`animateThumbnails`（网格）/ `animatePreview`（预览）两个独立设置

## 变更记录索引

- 2025-01-14：docs/.ai Init（首次记录）
