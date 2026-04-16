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

    void setButtonSize(int size);

    QString getCategoryName() const { return m_categoryName; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateIcon();

    QString m_categoryName;
    QString m_firstStickerPath;
    int m_buttonSize;
    QPixmap m_currentPixmap;
};

#endif // CATEGORYBUTTON_H
