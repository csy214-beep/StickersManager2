#ifndef CATEGORYBUTTON_H
#define CATEGORYBUTTON_H

#include <QPushButton>
#include <QString>
#include <QPixmap>

class QLabel;

class CategoryButton : public QPushButton {
    Q_OBJECT

public:
    explicit CategoryButton(const QString &categoryName,
                            int buttonSize,
                            QWidget *parent = nullptr);

    void setThumbnail(const QPixmap &pixmap);
    void setStickerCount(int count);
    void setShowName(bool show);
    void setShowCount(bool show);
    void setShowClock(bool show);

    QString getCategoryName() const { return m_categoryName; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateIcon();
    void updateOverlayLabels();

    QString m_categoryName;
    int m_buttonSize;
    bool m_showName = true;
    bool m_showCount = true;
    bool m_showClock = false;
    QPixmap m_currentPixmap;
    QLabel *m_nameLabel;
    QLabel *m_countLabel;
};

#endif // CATEGORYBUTTON_H
