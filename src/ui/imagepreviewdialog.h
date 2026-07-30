#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QMovie>
#include <QBuffer>
#include <QFutureWatcher>
#include <QVBoxLayout>

class ImagePreviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImagePreviewDialog(const QString &filePath, bool animatePreview, QWidget *parent = nullptr);
    ~ImagePreviewDialog();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void loadAndDisplayImage(const QString &filePath);
    void adjustDialogSize(const QSize &originalSize);

    QLabel *m_imageLabel;
    QPixmap m_originalPixmap;
    bool m_animatePreview = false;
    QMovie *m_movie = nullptr;
    QFutureWatcher<QByteArray> *m_animWatcher = nullptr;
    QSize m_displaySize;
};

#endif // IMAGEPREVIEWDIALOG_H
