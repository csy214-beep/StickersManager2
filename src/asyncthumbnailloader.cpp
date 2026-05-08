#include "asyncthumbnailloader.h"
#include "imageloader.h"
#include <QDebug>
#include <QPainter>
#include <QFileInfo>

AsyncThumbnailLoader::AsyncThumbnailLoader(QObject *parent)
    : QObject(parent)
      , m_threadPool(new QThreadPool(this)) {
    // 设置线程池
    m_threadPool->setMaxThreadCount(2); // 减少线程数，避免资源竞争
}

AsyncThumbnailLoader::~AsyncThumbnailLoader() {
    cancelAll();
    m_threadPool->waitForDone(1000); // 等待最多1秒
}

void AsyncThumbnailLoader::loadThumbnail(const QString &filePath, const QSize &targetSize) {
    QMutexLocker locker(&m_mutex);

    // 如果已经在加载，跳过
    if (m_pendingLoads.contains(filePath)) {
        qDebug() << "Thumbnail already in loading queue, skipping:" << filePath;
        return;
    }

    m_pendingLoads.insert(filePath);

    // 创建并启动加载任务
    LoadTask *task = new LoadTask(filePath, targetSize, this);
    m_threadPool->start(task);
    qDebug() << "Starting async thumbnail load:" << filePath << "Target size:" << targetSize;
}

void AsyncThumbnailLoader::loadThumbnails(const QVector<QPair<QString, QSize> > &thumbnails) {
    for (const auto &item: thumbnails) {
        loadThumbnail(item.first, item.second);
    }
}

void AsyncThumbnailLoader::cancelLoad(const QString &filePath) {
    QMutexLocker locker(&m_mutex);
    m_pendingLoads.remove(filePath);
    qDebug() << "Canceling thumbnail load:" << filePath;
}

void AsyncThumbnailLoader::cancelAll() {
    QMutexLocker locker(&m_mutex);
    m_pendingLoads.clear();
    m_threadPool->clear(); // 清除队列中的任务
    qDebug() << "Canceling all thumbnail load tasks";
}

void AsyncThumbnailLoader::setMaxThreadCount(int count) {
    m_threadPool->setMaxThreadCount(qMax(1, qMin(count, 4))); // 限制在1-4个线程
}

void AsyncThumbnailLoader::handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    QMutexLocker locker(&m_mutex);

    // 检查是否还在加载列表中
    if (!m_pendingLoads.contains(filePath)) {
        qDebug() << "Thumbnail canceled or timed out, ignoring:" << filePath;
        return;
    }

    m_pendingLoads.remove(filePath);

    if (pixmap.isNull()) {
        qWarning() << "Thumbnail load failed:" << filePath;
        emit loadFinished(filePath, false);
    } else {
        qDebug() << "Thumbnail load complete:" << filePath << "Size:" << pixmap.size();
        emit thumbnailLoaded(filePath, pixmap);
        emit loadFinished(filePath, true);
    }
}

// LoadTask 实现
AsyncThumbnailLoader::LoadTask::LoadTask(const QString &filePath, const QSize &targetSize,
                                         QObject *receiver)
    : m_filePath(filePath)
      , m_targetSize(targetSize)
      , m_receiver(receiver) {
    setAutoDelete(true);
}

void AsyncThumbnailLoader::LoadTask::run() {
    QImage image = AsyncThumbnailLoader::loadThumbnailInternal(m_filePath, m_targetSize);
    // 检查接收者是否仍然有效
    if (!m_receiver) {
        qWarning() << "Receiver destroyed, cannot send thumbnail:" << m_filePath;
        return;
    }

    // 使用信号槽机制将结果传回主线程
    AsyncThumbnailLoader *loader = qobject_cast<AsyncThumbnailLoader *>(m_receiver);
    if (loader) {
        // 使用 QueuedConnection 确保在主线程处理
        QString filePathCopy = m_filePath;
        QImage imageCopy = image;
        QMetaObject::invokeMethod(loader, [loader, filePathCopy, imageCopy]()
                                  {
            QPixmap pixmap = QPixmap::fromImage(imageCopy);
            loader->handleThumbnailLoaded(filePathCopy, pixmap); }, Qt::QueuedConnection);
    } else {
        qWarning() << "Cannot cast receiver to AsyncThumbnailLoader:" << m_filePath;
    }
}

QImage AsyncThumbnailLoader::loadThumbnailInternal(const QString &filePath, const QSize &targetSize) {
    // 检查文件是否还存在（可能在加载过程中被删除）
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        return QImage();
    }

    // 使用 ImageLoader 加载图像
    QImage image = ImageLoader::loadImage(filePath);
    if (image.isNull()) {
        qWarning() << "Cannot load image:" << filePath;

        // 创建错误占位图
        QImage placeholder(targetSize, QImage::Format_ARGB32);
        placeholder.fill(QColor(240, 240, 240));
        QPainter painter(&placeholder);
        painter.setPen(QColor(180, 180, 180));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "Load Failed");

        return placeholder;
    }

    // 创建缩略图
    QImage thumbnail = image.scaled(targetSize,
                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);

    qDebug() << "Thumbnail generation complete:" << filePath << "Original size:" << image.size() << "Thumbnail size:" << thumbnail.size();

    return thumbnail;
}
