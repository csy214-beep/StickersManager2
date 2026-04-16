#include "thumbnailcache.h"
#include "asyncthumbnailloader.h"
#include <QDebug>
#include <QPainter>

ThumbnailCache::ThumbnailCache(int maxSize, QObject *parent)
    : QObject(parent)
      , m_cache(maxSize)
      , m_asyncLoader(new AsyncThumbnailLoader(this)) {
    // 连接信号
    connect(m_asyncLoader, &AsyncThumbnailLoader::thumbnailLoaded,
            this, &ThumbnailCache::onAsyncThumbnailLoaded);

    qDebug() << "缩略图缓存初始化，最大大小:" << maxSize;
}

ThumbnailCache::~ThumbnailCache() {
    cancelAllLoads();
    clear();
}

QPixmap ThumbnailCache::get(const QString &key) {
    QMutexLocker locker(&m_cacheMutex);
    if (m_cache.contains(key)) {
        qDebug() << "从缓存获取缩略图:" << key;
        return *m_cache.object(key);
    }
    return QPixmap();
}

void ThumbnailCache::loadThumbnailAsync(const QString &imagePath, const QSize &targetSize) {
    // 先检查缓存，直接用文件路径作为键
    QPixmap cached;
    {
        QMutexLocker locker(&m_cacheMutex);
        if (m_cache.contains(imagePath))
        {
            cached = *m_cache.object(imagePath);
        }
    }

    if (!cached.isNull()) {
        // 缓存命中，直接发射信号
        qDebug() << "缩略图缓存命中:" << imagePath;
        emit thumbnailReady(imagePath, cached);
        return;
    }

    // 异步加载
    qDebug() << "请求异步加载缩略图:" << imagePath;
    m_asyncLoader->loadThumbnail(imagePath, targetSize);
}

void ThumbnailCache::loadThumbnailsAsync(const QVector<QPair<QString, QSize> > &thumbnails) {
    m_asyncLoader->loadThumbnails(thumbnails);
}

void ThumbnailCache::cancelLoad(const QString &imagePath) {
    m_asyncLoader->cancelLoad(imagePath);
}

void ThumbnailCache::cancelAllLoads() {
    m_asyncLoader->cancelAll();
}

void ThumbnailCache::clear() {
    QMutexLocker locker(&m_cacheMutex);
    m_cache.clear();
    qDebug() << "缩略图缓存已清空";
}

QPixmap ThumbnailCache::createThumbnail(const QString &imagePath, const QSize &targetSize) {
    QString cacheKey = getCacheKey(imagePath, targetSize);

    // 先检查缓存
    {
        QMutexLocker locker(&m_cacheMutex);
        if (m_cache.contains(cacheKey)) {
            return *m_cache.object(cacheKey);
        }
    }

    // 同步加载（不建议在主线程使用）
    qWarning() << "警告：在主线程同步加载缩略图，可能导致界面卡顿:" << imagePath;

    // 这里可以调用 ImageLoader 同步加载，但会阻塞UI
    // 暂时返回空，让异步加载器处理
    return QPixmap();
}

void ThumbnailCache::onAsyncThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    qDebug() << "缩略图缓存收到异步加载结果:" << filePath << "大小:" << pixmap.size();

    if (pixmap.isNull()) {
        qWarning() << "加载的缩略图为空，创建占位图:" << filePath;

        // 创建一个错误占位图
        QPixmap errorPixmap(100, 100);
        errorPixmap.fill(QColor(255, 230, 230));
        QPainter painter(&errorPixmap);
        painter.setPen(QColor(255, 100, 100));
        painter.drawText(errorPixmap.rect(), Qt::AlignCenter, "错误");
        emit thumbnailReady(filePath, errorPixmap);
        return;
    }

    // 将图片放入缓存，使用文件路径作为键
    {
        QMutexLocker locker(&m_cacheMutex);
        m_cache.insert(filePath, new QPixmap(pixmap), pixmap.width() * pixmap.height() * 4);
    }

    qDebug() << "发射缩略图就绪信号:" << filePath;
    emit thumbnailReady(filePath, pixmap);
}

QString ThumbnailCache::getCacheKey(const QString &imagePath, const QSize &targetSize) {
    return QString("%1_%2x%3").arg(imagePath).arg(targetSize.width()).arg(targetSize.height());
}
