#include "asyncthumbnailloader.h"
#include "imageloader.h"
#include <QDebug>
#include <QPainter>
#include <QFileInfo>

AsyncThumbnailLoader::AsyncThumbnailLoader(QObject *parent)
    : QObject(parent)
      , m_threadPool(new QThreadPool(this)) {
    m_threadPool->setMaxThreadCount(2);
}

AsyncThumbnailLoader::~AsyncThumbnailLoader() {
    cancelAll();
    m_threadPool->waitForDone(1000);
}

void AsyncThumbnailLoader::loadThumbnail(const QString &filePath, const QSize &targetSize) {
    QMutexLocker locker(&m_mutex);

    if (m_pendingLoads.contains(filePath)) {
        return;
    }

    m_pendingLoads.insert(filePath);

    LoadTask *task = new LoadTask(filePath, targetSize, this);
    m_threadPool->start(task);
}

void AsyncThumbnailLoader::loadThumbnails(const QVector<QPair<QString, QSize> > &thumbnails) {
    for (const auto &item: thumbnails) {
        loadThumbnail(item.first, item.second);
    }
}

void AsyncThumbnailLoader::cancelAll() {
    QMutexLocker locker(&m_mutex);
    m_pendingLoads.clear();
    m_threadPool->clear();
}

void AsyncThumbnailLoader::handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    QMutexLocker locker(&m_mutex);

    if (!m_pendingLoads.contains(filePath)) {
        return;
    }

    m_pendingLoads.remove(filePath);

    if (!pixmap.isNull()) {
        emit thumbnailLoaded(filePath, pixmap);
    }
}

AsyncThumbnailLoader::LoadTask::LoadTask(const QString &filePath, const QSize &targetSize,
                                         QObject *receiver)
    : m_filePath(filePath)
      , m_targetSize(targetSize)
      , m_receiver(receiver) {
    setAutoDelete(true);
}

void AsyncThumbnailLoader::LoadTask::run() {
    QImage image = AsyncThumbnailLoader::loadThumbnailInternal(m_filePath, m_targetSize);
    if (!m_receiver) {
        return;
    }

    AsyncThumbnailLoader *loader = qobject_cast<AsyncThumbnailLoader *>(m_receiver);
    if (loader) {
        QMetaObject::invokeMethod(loader, [loader, filePath = m_filePath, image]()
                                  {
            QPixmap pixmap = QPixmap::fromImage(image);
            loader->handleThumbnailLoaded(filePath, pixmap); }, Qt::QueuedConnection);
    }
}

QImage AsyncThumbnailLoader::loadThumbnailInternal(const QString &filePath, const QSize &targetSize) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return QImage();
    }

    QImage image = ImageLoader::loadImage(filePath);
    if (image.isNull()) {
        QImage placeholder(targetSize, QImage::Format_ARGB32);
        placeholder.fill(QColor(240, 240, 240));
        QPainter painter(&placeholder);
        painter.setPen(QColor(180, 180, 180));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "Load Failed");
        return placeholder;
    }

    QImage thumbnail = image.scaled(targetSize,
                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return thumbnail;
}
