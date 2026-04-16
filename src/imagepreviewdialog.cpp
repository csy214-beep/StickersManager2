#include "imagepreviewdialog.h"
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include "imageloader.h"

ImagePreviewDialog::ImagePreviewDialog(const QString &filePath, QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("ImagePreviewDialog { background-color: rgba(0, 0, 0, 200); }");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_imageLabel);

    loadAndDisplayImage(filePath);
}



void ImagePreviewDialog::loadAndDisplayImage(const QString &filePath) {
    QImage image = ImageLoader::loadImage(filePath);
    m_originalPixmap = QPixmap::fromImage(image);

    if (m_originalPixmap.isNull()) {
        qWarning() << "Failed to load image for preview:" << filePath;
        m_imageLabel->setText("Failed to load image");
        return;
    }

    adjustDialogSize();
}

void ImagePreviewDialog::adjustDialogSize() {
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    int maxWidth = static_cast<int>(screenGeometry.width() * 0.8);
    int maxHeight = static_cast<int>(screenGeometry.height() * 0.8);

    QPixmap scaledPixmap = m_originalPixmap.scaled(
        maxWidth, maxHeight,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    m_imageLabel->setPixmap(scaledPixmap);

    QSize dialogSize = scaledPixmap.size() + QSize(40, 40);
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
