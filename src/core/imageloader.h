#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QImage>
#include <QString>
#include <QSize>
#include <QSet>

class ImageLoader {
public:
    enum class Format {
        Unknown,
        PNG, JPEG, GIF, BMP, TIFF, WEBP,
        PSD, HDR, TGA,
        ICO, SVG, HEIC, AVIF
    };

    static QImage loadImage(const QString &filePath);
    static QImage loadImageScaled(const QString &filePath, const QSize &targetSize);
    static bool isFormatSupported(const QString &filePath);
    static bool isAnimated(const QString &filePath);
    static QStringList getSupportedExtensions();

private:
    static QImage loadWithStb(const QString &filePath);
    static QImage loadWithQt(const QString &filePath);
    static Format detectFormat(const QString &filePath);
    static QImage createQImageFromData(unsigned char *data, int width, int height, int channels);
    static QSet<QString> s_animatedCache;
    static QSet<QString> s_staticCache;
};

#endif // IMAGELOADER_H
