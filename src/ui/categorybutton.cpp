#include "categorybutton.h"
#include <QIcon>
#include <QDebug>
#include <QResizeEvent>
#include <QStyle>

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

    QColor bg = palette().color(QPalette::Button);
    QColor hl = palette().color(QPalette::Highlight);

    setStyleSheet(QString(R"(
        CategoryButton {
            background-color: %1;
            border: 2px solid %2;
            border-radius: 8px;
        }
        CategoryButton:hover {
            border: 2px solid %3;
            background-color: %4;
        }
        CategoryButton:pressed {
            background-color: %3;
            border: 2px solid %3;
        }
    )")
        .arg(bg.name())
        .arg(bg.darker(130).name())
        .arg(hl.name())
        .arg(hl.lighter(185).name()));
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
