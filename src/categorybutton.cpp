#include "categorybutton.h"
#include <QIcon>
#include <QDebug>
#include <QResizeEvent>

CategoryButton::CategoryButton(const QString &categoryName,
                               const QString &firstStickerPath,
                               int buttonSize,
                               QWidget *parent)
    : QPushButton(parent),
      m_categoryName(categoryName),
      m_buttonSize(buttonSize)
{
    setFixedSize(buttonSize, buttonSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setToolTip(categoryName);

    setStyleSheet(R"(
        CategoryButton {
            background-color: #ffffff;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
        }
        CategoryButton:hover {
            border: 2px solid #409eff;
            background-color: #e8f4f8;
        }
        CategoryButton:pressed {
            background-color: #409eff;
            border: 2px solid #409eff;
        }
    )");
}

void CategoryButton::setThumbnail(const QPixmap &pixmap)
{
    if (!pixmap.isNull())
    {
        m_currentPixmap = pixmap;
        updateIcon();
    }
}

void CategoryButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    if (!m_currentPixmap.isNull())
    {
        updateIcon();
    }
}

void CategoryButton::updateIcon()
{
    if (m_currentPixmap.isNull())
        return;

    int iconSize = m_buttonSize - 10;
    if (iconSize < 10)
        iconSize = 10;

    QPixmap scaledPixmap = m_currentPixmap.scaled(
        iconSize,
        iconSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    setIcon(QIcon(scaledPixmap));
    setIconSize(QSize(iconSize, iconSize));
}
