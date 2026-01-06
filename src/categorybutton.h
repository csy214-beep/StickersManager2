#ifndef CATEGORYBUTTON_H
#define CATEGORYBUTTON_H

#include <QPushButton>
#include <QString>
#include <QPixmap>

class CategoryButton : public QPushButton {
    Q_OBJECT

public:
    explicit CategoryButton(const QString &categoryName,
                            const QString &firstStickerPath,
                            int buttonSize,
                            QWidget *parent = nullptr);

    void setThumbnail(const QPixmap &pixmap);

    QString getCategoryName() const { return m_categoryName; }

private:
    QString m_categoryName;
    QString m_firstStickerPath;
    int m_buttonSize;
};

#endif // CATEGORYBUTTON_H
