#include "imageloader.h"
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QDebug>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996 4100 4244)
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_FAILURE_USERMSG
#include "thirdparty/stb/stb_image.h"
#endif

#ifndef STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_resize2.h"
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

QImage ImageLoader::loadImage(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return QImage();
    }

    Format format = detectFormat(filePath);

    switch (format) {
        case Format::SVG:
        case Format::ICO:
            return loadWithQt(filePath);
        default:
            QImage result = loadWithStb(filePath);
            if (!result.isNull()) {
                return result;
            }
            return loadWithQt(filePath);
    }
}

ImageLoader::Format ImageLoader::detectFormat(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    static QHash<QString, Format> formatMap = {
        {"png", Format::PNG},
        {"jpg", Format::JPEG}, {"jpeg", Format::JPEG},
        {"gif", Format::GIF},
        {"bmp", Format::BMP},
        {"tiff", Format::TIFF}, {"tif", Format::TIFF},
        {"webp", Format::WEBP},
        {"psd", Format::PSD},
        {"hdr", Format::HDR},
        {"tga", Format::TGA},
        {"ico", Format::ICO},
        {"svg", Format::SVG},
        {"heic", Format::HEIC}, {"heif", Format::HEIC},
        {"avif", Format::AVIF}
    };

    return formatMap.value(suffix, Format::Unknown);
}

bool ImageLoader::isFormatSupported(const QString &filePath) {
    Format format = detectFormat(filePath);
    return format != Format::Unknown;
}

bool ImageLoader::isAnimated(const QString &filePath) {
    Format format = detectFormat(filePath);
    if (format == Format::GIF) return true;

    QImageReader reader(filePath);
    return reader.imageCount() > 1;
}

QStringList ImageLoader::getSupportedExtensions() {
    return {
        "png", "jpg", "jpeg", "gif", "bmp",
        "tiff", "tif", "webp", "psd", "hdr",
        "tga", "ico", "svg", "heic", "heif", "avif"
    };
}

QImage ImageLoader::loadWithStb(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QImage();
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        return QImage();
    }

    int width, height, channels;
    unsigned char *imageData = stbi_load_from_memory(
        reinterpret_cast<const unsigned char *>(data.constData()),
        data.size(),
        &width, &height, &channels,
        0
    );

    if (!imageData) {
        qWarning() << "stb_image cannot load image:" << filePath << stbi_failure_reason();
        return QImage();
    }

    QImage result = createQImageFromData(imageData, width, height, channels);
    stbi_image_free(imageData);

    return result;
}

QImage ImageLoader::loadWithQt(const QString &filePath) {
    QImage image;
    if (!image.load(filePath)) {
        return QImage();
    }

    if (image.format() == QImage::Format_Invalid) {
        return QImage();
    }

    if (image.format() != QImage::Format_ARGB32 &&
        image.format() != QImage::Format_RGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    return image;
}

QImage ImageLoader::createQImageFromData(unsigned char *data, int width, int height, int channels) {
    if (!data || width <= 0 || height <= 0) {
        return QImage();
    }

    switch (channels) {
        case 1:
        {
            QImage image(width, height, QImage::Format_Grayscale8);
            for (int y = 0; y < height; ++y) {
                memcpy(image.scanLine(y), data + y * width, width);
            }
            return image;
        }
        case 2:
        {
            QImage image(width, height, QImage::Format_ARGB32);
            for (int y = 0; y < height; ++y) {
                QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < width; ++x) {
                    unsigned char gray = data[y * width * 2 + x * 2];
                    unsigned char alpha = data[y * width * 2 + x * 2 + 1];
                    scanLine[x] = qRgba(gray, gray, gray, alpha);
                }
            }
            return image;
        }
        case 3:
        {
            QImage image(width, height, QImage::Format_RGB888);
            for (int y = 0; y < height; ++y) {
                memcpy(image.scanLine(y), data + y * width * 3, width * 3);
            }
            return image;
        }
        case 4:
        {
            QImage image(width, height, QImage::Format_ARGB32);
            for (int y = 0; y < height; ++y) {
                QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < width; ++x) {
                    int offset = y * width * 4 + x * 4;
                    scanLine[x] = qRgba(data[offset], data[offset + 1], data[offset + 2], data[offset + 3]);
                }
            }
            return image;
        }
        default:
            return QImage();
    }
}
