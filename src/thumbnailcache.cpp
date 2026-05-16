#include "thumbnailcache.h"
#include "asyncthumbnailloader.h"
#include <QDebug>
#include <QPainter>

ThumbnailCache::ThumbnailCache(int maxSize, QObject *parent)
    : QObject(parent)
      , m_cache(maxSize)
      , m_asyncLoader(new AsyncThumbnailLoader(this)) {
    connect(m_asyncLoader, &AsyncThumbnailLoader::thumbnailLoaded,
            this, &ThumbnailCache::onAsyncThumbnailLoaded);

    qDebug() << "Thumbnail cache initialized, max size:" << maxSize;
}

ThumbnailCache::~ThumbnailCache() {
    cancelAllLoads();
    clear();
}

QPixmap ThumbnailCache::get(const QString &key) {
    QMutexLocker locker(&m_cacheMutex);
    if (m_cache.contains(key)) {
        return *m_cache.object(key);
    }
    return QPixmap();
}

void ThumbnailCache::loadThumbnailAsync(const QString &imagePath, const QSize &targetSize) {
    QPixmap cached;
    {
        QMutexLocker locker(&m_cacheMutex);
        if (m_cache.contains(imagePath))
        {
            cached = *m_cache.object(imagePath);
        }
    }

    if (!cached.isNull()) {
        emit thumbnailReady(imagePath, cached);
        return;
    }

    m_asyncLoader->loadThumbnail(imagePath, targetSize);
}

void ThumbnailCache::loadThumbnailsAsync(const QVector<QPair<QString, QSize> > &thumbnails) {
    m_asyncLoader->loadThumbnails(thumbnails);
}

void ThumbnailCache::cancelAllLoads() {
    m_asyncLoader->cancelAll();
}

void ThumbnailCache::clear() {
    QMutexLocker locker(&m_cacheMutex);
    m_cache.clear();
}

void ThumbnailCache::onAsyncThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    if (pixmap.isNull()) {
        QPixmap errorPixmap(100, 100);
        errorPixmap.fill(QColor(255, 230, 230));
        QPainter painter(&errorPixmap);
        painter.setPen(QColor(255, 100, 100));
        painter.drawText(errorPixmap.rect(), Qt::AlignCenter, "Error");
        emit thumbnailReady(filePath, errorPixmap);
        return;
    }

    {
        QMutexLocker locker(&m_cacheMutex);
        m_cache.insert(filePath, new QPixmap(pixmap), pixmap.width() * pixmap.height() * 4);
    }

    emit thumbnailReady(filePath, pixmap);
}
