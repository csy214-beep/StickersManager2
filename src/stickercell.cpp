#include "stickercell.h"

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QResizeEvent>
#include <QFileInfo>
#include <QClipboard>
#include <QMimeData>
#include  "tray.h"

StickerCell::StickerCell(const QString &filePath, int cellSize, QWidget *parent)
    : QFrame(parent)
      , m_filePath(filePath)
      , m_cellSize(cellSize)
      , m_isHighlighted(false)
      , m_hasRealThumbnail(false) {
    setFixedSize(cellSize, cellSize);
    setFrameShape(QFrame::Box);
    setStyleSheet("QFrame { background-color: #f5f5f5; border: 2px solid #e0e0e0; }");

    // 图片标签
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(false); // 让Qt自动处理缩放

    // 使用布局确保标签填满整个单元格
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(0);
    layout->addWidget(m_imageLabel);

    // 初始设置为占位图
    setPlaceholder();
}

StickerCell::~StickerCell() {
    // 清理资源
}

void StickerCell::setThumbnail(const QPixmap &pixmap) {
    if (pixmap.isNull()) {
        qWarning() << "尝试设置空的缩略图到单元格:" << m_filePath;
        return;
    }

    m_currentPixmap = pixmap;
    m_hasRealThumbnail = true;

    // 直接设置到标签，让Qt处理缩放
    m_imageLabel->setPixmap(pixmap);

    // 强制更新显示
    m_imageLabel->update();
    update();
    qDebug() << "StickerCell: 已设置缩略图，大小:" << pixmap.size();
}

void StickerCell::setPlaceholder() {
    // 创建一个简单的占位图
    // QPixmap placeholder(m_cellSize - 10, m_cellSize - 10);
    // placeholder.fill(Qt::transparent); // 使用透明背景，避免覆盖
    //
    // m_imageLabel->setPixmap(placeholder);
    m_imageLabel->clear();
    m_imageLabel->setText("..."); // 或者显示加载中
    m_hasRealThumbnail = false;
}

void StickerCell::clearHighlight() {
    m_isHighlighted = false;
    setStyleSheet("QFrame { background-color: #f5f5f5; border: 2px solid #e0e0e0; }");
}

void StickerCell::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    // 单击高亮
    m_isHighlighted = true;
    setStyleSheet("QFrame { background-color: #e3f2fd; border: 2px solid #2196f3; }");
    emit clicked(m_filePath);
}

void StickerCell::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);

    // 双击复制文件到剪贴板
    QFile file(m_filePath);

    if (!file.exists()) {
        qWarning() << "文件不存在:" << m_filePath;
        TrayIcon::showMessage("Warning", "文件不存在:" + m_filePath);
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard) {
        // 创建MIME数据，支持文件粘贴操作
        QMimeData *mimeData = new QMimeData();
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(m_filePath));
        mimeData->setUrls(urls);
        mimeData->setText(m_filePath); // 文本格式作为备份

        clipboard->setMimeData(mimeData);
        qDebug() << "已复制文件到剪贴板:" << m_filePath;
        TrayIcon::showMessage("Info", "已复制文件到剪贴板:" + m_filePath);
    }

    // 仍然发射双击信号（如果其他地方需要）
    emit doubleClicked(m_filePath);
}

void StickerCell::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);

    // 如果有真实缩略图，重新设置一次以确保正确显示
    if (m_hasRealThumbnail && !m_currentPixmap.isNull()) {
        m_imageLabel->setPixmap(m_currentPixmap);
    }
}
