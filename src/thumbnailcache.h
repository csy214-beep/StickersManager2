#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include <QObject>
#include <QPixmap>
#include <QCache>
#include <QDir>
#include <QMutex>

class AsyncThumbnailLoader;

class ThumbnailCache : public QObject {
    Q_OBJECT

public:
    explicit ThumbnailCache(int maxSize = 200, QObject *parent = nullptr);

    ~ThumbnailCache();

    // 同步获取（如果缓存中有）
    QPixmap get(const QString &key);

    // 异步加载缩略图
    void loadThumbnailAsync(const QString &imagePath, const QSize &targetSize);

    // 批量异步加载
    void loadThumbnailsAsync(const QVector<QPair<QString, QSize> > &thumbnails);

    // 取消加载
    void cancelLoad(const QString &imagePath);

    void cancelAllLoads();

    void clear();

    // 生成缩略图（同步，用于预加载）
    QPixmap createThumbnail(const QString &imagePath, const QSize &targetSize);

    // 统计数据
    int size() const { return m_cache.size(); }
    int maxSize() const { return m_cache.maxCost(); }
    void setMaxSize(int size) { m_cache.setMaxCost(size); }

signals:
    void thumbnailReady(const QString &imagePath, const QPixmap &thumbnail);

private slots:
    void onAsyncThumbnailLoaded(const QString &filePath, const QPixmap &pixmap);

private:
    QCache<QString, QPixmap> m_cache;
    QMutex m_cacheMutex;
    AsyncThumbnailLoader *m_asyncLoader;

    QString getCacheKey(const QString &imagePath, const QSize &targetSize);
};

#endif // THUMBNAILCACHE_H
