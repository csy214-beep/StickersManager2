#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

class ImagePreviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImagePreviewDialog(const QString &filePath, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void loadAndDisplayImage(const QString &filePath);
    void adjustDialogSize();

    QLabel *m_imageLabel;
    QPixmap m_originalPixmap;
};

#endif // IMAGEPREVIEWDIALOG_H
