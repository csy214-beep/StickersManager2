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
        qDebug() << "缩略图已在加载队列中，跳过:" << filePath;
        return;
    }

    m_pendingLoads.insert(filePath);

    // 创建并启动加载任务
    LoadTask *task = new LoadTask(filePath, targetSize, this);
    m_threadPool->start(task);
    qDebug() << "开始异步加载缩略图:" << filePath << "目标大小:" << targetSize;
}

void AsyncThumbnailLoader::loadThumbnails(const QVector<QPair<QString, QSize> > &thumbnails) {
    for (const auto &item: thumbnails) {
        loadThumbnail(item.first, item.second);
    }
}

void AsyncThumbnailLoader::cancelLoad(const QString &filePath) {
    QMutexLocker locker(&m_mutex);
    m_pendingLoads.remove(filePath);
    qDebug() << "取消加载缩略图:" << filePath;
}

void AsyncThumbnailLoader::cancelAll() {
    QMutexLocker locker(&m_mutex);
    m_pendingLoads.clear();
    m_threadPool->clear(); // 清除队列中的任务
    qDebug() << "取消所有缩略图加载任务";
}

void AsyncThumbnailLoader::setMaxThreadCount(int count) {
    m_threadPool->setMaxThreadCount(qMax(1, qMin(count, 4))); // 限制在1-4个线程
}

void AsyncThumbnailLoader::handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap) {
    QMutexLocker locker(&m_mutex);

    // 检查是否还在加载列表中
    if (!m_pendingLoads.contains(filePath)) {
        qDebug() << "缩略图已取消或超时，忽略:" << filePath;
        return;
    }

    m_pendingLoads.remove(filePath);

    if (pixmap.isNull()) {
        qWarning() << "缩略图加载失败:" << filePath;
        emit loadFinished(filePath, false);
    } else {
        qDebug() << "缩略图加载完成:" << filePath << "大小:" << pixmap.size();
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
        qWarning() << "接收者已销毁，无法发送缩略图:" << m_filePath;
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
        qWarning() << "无法将接收者转换为 AsyncThumbnailLoader:" << m_filePath;
    }
}

QImage AsyncThumbnailLoader::loadThumbnailInternal(const QString &filePath, const QSize &targetSize) {
    // 检查文件是否还存在（可能在加载过程中被删除）
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "文件不存在:" << filePath;
        return QImage();
    }

    // 使用 ImageLoader 加载图像
    QImage image = ImageLoader::loadImage(filePath);
    if (image.isNull()) {
        qWarning() << "无法加载图像:" << filePath;

        // 创建错误占位图
        QImage placeholder(targetSize, QImage::Format_ARGB32);
        placeholder.fill(QColor(240, 240, 240));
        QPainter painter(&placeholder);
        painter.setPen(QColor(180, 180, 180));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "加载失败");

        return placeholder;
    }

    // 创建缩略图
    QImage thumbnail = image.scaled(targetSize,
                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);

    qDebug() << "缩略图生成完成:" << filePath << "原始大小:" << image.size() << "缩略图大小:" << thumbnail.size();

    return thumbnail;
}
