#include "imagepreviewdialog.h"
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImageReader>
#include <QFile>
#include <QtConcurrent>
#include <QDebug>
#include "imageloader.h"

ImagePreviewDialog::ImagePreviewDialog(const QString &filePath, bool animatePreview, QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_animatePreview(animatePreview) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setStyleSheet("ImagePreviewDialog { background-color: rgba(0, 0, 0, 200); }");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_imageLabel);

    loadAndDisplayImage(filePath);
}

ImagePreviewDialog::~ImagePreviewDialog() {
    if (m_animWatcher) {
        m_animWatcher->cancel();
    }
    if (m_movie) {
        m_movie->stop();
    }
}

void ImagePreviewDialog::loadAndDisplayImage(const QString &filePath) {
    if (m_animatePreview && ImageLoader::isAnimated(filePath)) {
        QImageReader reader(filePath);
        QSize frameSize = reader.size();
        if (!frameSize.isValid()) {
            m_imageLabel->setText("Failed to load image");
            return;
        }

        adjustDialogSize(frameSize);
        m_imageLabel->setText("Loading...");

        m_animWatcher = new QFutureWatcher<QByteArray>(this);
        connect(m_animWatcher, &QFutureWatcher<QByteArray>::finished, this, [this]() {
            if (!m_animWatcher || m_animWatcher->isCanceled()) return;
            QByteArray data = m_animWatcher->result();
            m_animWatcher->deleteLater();
            m_animWatcher = nullptr;
            if (data.isEmpty()) {
                m_imageLabel->setText("Failed to load");
                return;
            }
            auto *buffer = new QBuffer();
            buffer->setData(data);
            m_movie = new QMovie(buffer);
            buffer->setParent(m_movie);
            connect(m_movie, &QMovie::frameChanged, this, [this](int) {
                if (!m_movie) return;
                QPixmap framePix = m_movie->currentPixmap();
                if (framePix.isNull()) return;
                m_imageLabel->setPixmap(framePix.scaled(
                    m_displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            });
            m_movie->start();
        });
        m_animWatcher->setFuture(QtConcurrent::run([filePath]() -> QByteArray {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) return {};
            return file.readAll();
        }));
        return;
    }

    QImage image = ImageLoader::loadImage(filePath);
    m_originalPixmap = QPixmap::fromImage(image);

    if (m_originalPixmap.isNull()) {
        qWarning() << "Failed to load image for preview:" << filePath;
        m_imageLabel->setText("Failed to load image");
        return;
    }

    adjustDialogSize(m_originalPixmap.size());
    m_imageLabel->setPixmap(m_originalPixmap.scaled(
        m_displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ImagePreviewDialog::adjustDialogSize(const QSize &originalSize) {
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    int maxWidth = static_cast<int>(screenGeometry.width() * 0.8);
    int maxHeight = static_cast<int>(screenGeometry.height() * 0.8);

    m_displaySize = originalSize.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);

    QSize dialogSize = m_displaySize + QSize(40, 40);
    resize(dialogSize);

    QPoint center = screenGeometry.center() - rect().center();
    move(center);
}

void ImagePreviewDialog::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    accept();
}

void ImagePreviewDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape ||
        event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Space) {
        accept();
    } else {
        QDialog::keyPressEvent(event);
    }
}
