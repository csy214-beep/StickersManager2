#ifndef ASYNCTHUMBNAILLOADER_H
#define ASYNCTHUMBNAILLOADER_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <QMutex>
#include <QSet>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QPair>
#include <QVector>

class AsyncThumbnailLoader : public QObject {
    Q_OBJECT

public:
    explicit AsyncThumbnailLoader(QObject *parent = nullptr);

    ~AsyncThumbnailLoader();

    // 加载缩略图
    void loadThumbnail(const QString &filePath, const QSize &targetSize);

    // 批量加载
    void loadThumbnails(const QVector<QPair<QString, QSize> > &thumbnails);

    // 清除指定路径的加载任务
    void cancelLoad(const QString &filePath);

    // 清除所有加载任务
    void cancelAll();

    // 设置最大线程数
    void setMaxThreadCount(int count);

signals:
    void thumbnailLoaded(const QString &filePath, const QPixmap &pixmap);

    void loadFinished(const QString &filePath, bool success);

private slots:
    void handleThumbnailLoaded(const QString &filePath, const QPixmap &pixmap);

private:
    QThreadPool *m_threadPool;
    QMutex m_mutex;
    QSet<QString> m_pendingLoads; // 正在加载的文件

    // 内部加载任务
    class LoadTask : public QRunnable {
    public:
        LoadTask(const QString &filePath, const QSize &targetSize, QObject *receiver);

        void run() override;

    private:
        QString m_filePath;
        QSize m_targetSize;
        QObject *m_receiver; // 使用 QObject* 而不是 AsyncThumbnailLoader*
    };

    // 加载缩略图的实际函数（在后台线程运行）
    static QImage loadThumbnailInternal(const QString &filePath, const QSize &targetSize);
};

#endif // ASYNCTHUMBNAILLOADER_H
