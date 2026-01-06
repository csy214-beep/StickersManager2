#include "categorybutton.h"
#include <QIcon>
#include <QDebug>

CategoryButton::CategoryButton(const QString &categoryName,
                               const QString &firstStickerPath,
                               int buttonSize,
                               QWidget *parent)
    : QPushButton(parent)
      , m_categoryName(categoryName)
      , m_firstStickerPath(firstStickerPath)
      , m_buttonSize(buttonSize) {
    setFixedSize(buttonSize, buttonSize);
    setToolTip(categoryName);
    setStyleSheet(R"(
        QPushButton {
            background-color: #ffffff;
            border: 2px solid #e0e0e0;
            border-radius: 4px;
        }
        QPushButton:hover {
            border: 2px solid #2196f3;
            background-color: #e3f2fd;
        }
        QPushButton:pressed {
            background-color: #bbdefb;
        }
    )");
}

void CategoryButton::setThumbnail(const QPixmap &pixmap) {
    if (!pixmap.isNull()) {
        // 缩放并设置为图标
        QPixmap scaledPixmap = pixmap.scaled(
            m_buttonSize - 10,
            m_buttonSize - 10,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        setIcon(QIcon(scaledPixmap));
        setIconSize(QSize(m_buttonSize - 10, m_buttonSize - 10));
    }
}
