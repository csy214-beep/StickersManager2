#ifndef STICKERCELL_H
#define STICKERCELL_H

#include <QFrame>
#include <QString>
#include <QPixmap>

class QLabel;

class StickerCell : public QFrame {
    Q_OBJECT

public:
    explicit StickerCell(const QString &filePath, int cellSize, QWidget *parent = nullptr);

    ~StickerCell();

    QString getFilePath() const { return m_filePath; }

    void setThumbnail(const QPixmap &pixmap);

    void clearHighlight();

    // 设置占位图（只在构造函数中调用一次）
    void setPlaceholder();

signals:
    void clicked(const QString &filePath);

    void doubleClicked(const QString &filePath);

    void rightClicked(const QString &filePath);

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override; // 添加重绘事件

private:
    QString m_filePath;
    QLabel *m_imageLabel;
    bool m_isHighlighted;
    int m_cellSize;
    QPixmap m_currentPixmap; // 保存当前图片
    bool m_hasRealThumbnail; // 标记是否有真实缩略图
};

#endif // STICKERCELL_H
