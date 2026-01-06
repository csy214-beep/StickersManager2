#include "imageloader.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QBuffer>
#include <QtGlobal>

// 禁用 stb_image 的警告
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996 4100 4244)
#endif

// 只定义 STB_IMAGE_IMPLEMENTATION 一次
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

// 检查是否有 libwebp
#ifdef HAVE_WEBP
#include <webp/decode.h>
#include <webp/demux.h>
#endif

#include <memory>

std::function<void(int)> ImageLoader::progressCallback = nullptr;

QImage ImageLoader::loadImage(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "文件不存在:" << filePath;
        return QImage();
    }

    Format format = detectFormat(filePath);

    // 根据格式选择加载器
    switch (format) {
        case Format::WEBP:
#ifdef HAVE_WEBP
            return loadWebP(filePath);
#else
            // 如果没有 libwebp，尝试用 stb_image 加载
            // stb_image 从 2.27 开始支持 webp
            break;
#endif
        case Format::SVG:
        case Format::ICO:
            // Qt 支持这些格式
            return loadWithQt(filePath);
        default:
            // 使用 stb_image 加载常见格式
            QImage result = loadWithStb(filePath);
            if (!result.isNull()) {
                return result;
            }
            // 如果 stb_image 失败，尝试 Qt
            return loadWithQt(filePath);
    }

    return loadWithStb(filePath);
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

QStringList ImageLoader::getSupportedExtensions() {
    return {
        "png", "jpg", "jpeg", "gif", "bmp",
        "tiff", "tif", "webp", "psd", "hdr",
        "tga", "ico", "svg", "heic", "heif", "avif"
    };
}

QString ImageLoader::getFormatDescription(Format format) {
    static QHash<Format, QString> descriptions = {
        {Format::PNG, "Portable Network Graphics"},
        {Format::JPEG, "JPEG Image"},
        {Format::GIF, "Graphics Interchange Format"},
        {Format::BMP, "Bitmap Image"},
        {Format::TIFF, "Tagged Image File Format"},
        {Format::WEBP, "Google WebP Image"},
        {Format::PSD, "Adobe Photoshop Document"},
        {Format::HDR, "High Dynamic Range Image"},
        {Format::TGA, "Truevision TGA Image"},
        {Format::ICO, "Windows Icon"},
        {Format::SVG, "Scalable Vector Graphics"},
        {Format::HEIC, "High Efficiency Image Format"},
        {Format::AVIF, "AV1 Image File Format"}
    };

    return descriptions.value(format, "Unknown Format");
}

void ImageLoader::setProgressCallback(std::function<void(int)> callback) {
    progressCallback = callback;
}

QImage ImageLoader::loadWithStb(const QString &filePath) {
    // 读取文件到内存
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath;
        return QImage();
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        qWarning() << "文件为空:" << filePath;
        return QImage();
    }

    // 调用进度回调
    if (progressCallback) {
        progressCallback(10);
    }

    int width, height, channels;
    unsigned char *imageData = stbi_load_from_memory(
        reinterpret_cast<const unsigned char *>(data.constData()),
        data.size(),
        &width, &height, &channels,
        0
    );

    if (progressCallback) {
        progressCallback(50);
    }

    if (!imageData) {
        qWarning() << "stb_image 无法加载图像:" << filePath << stbi_failure_reason();
        return QImage();
    }

    QImage result = createQImageFromData(imageData, width, height, channels);

    if (progressCallback) {
        progressCallback(100);
    }

    stbi_image_free(imageData);

    return result;
}

QImage ImageLoader::loadWithQt(const QString &filePath) {
    // Qt 内置支持的格式
    QImage image;
    if (!image.load(filePath)) {
        qWarning() << "Qt 无法加载图像:" << filePath;
        return QImage();
    }

    // 确保图像有合适的格式
    if (image.format() == QImage::Format_Invalid) {
        qWarning() << "Qt 加载的图像格式无效:" << filePath;
        return QImage();
    }

    // 转换为标准格式以便处理
    if (image.format() != QImage::Format_ARGB32 &&
        image.format() != QImage::Format_RGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    return image;
}

#ifdef HAVE_WEBP
QImage ImageLoader::loadWebP(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QImage();
    }

    QByteArray data = file.readAll();
    file.close();

    // 检测是否为动画 WebP
    WebPData webpData;
    webpData.bytes = reinterpret_cast<const uint8_t *>(data.constData());
    webpData.size = data.size();

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    dec_options.color_mode = MODE_BGRA;

    WebPAnimDecoder *dec = WebPAnimDecoderNew(&webpData, &dec_options);
    if (!dec) {
        // 不是动画，尝试作为静态图像加载
        int width, height;
        uint8_t *webpData = WebPDecodeBGRA(
            reinterpret_cast<const uint8_t *>(data.constData()),
            data.size(),
            &width, &height
        );

        if (!webpData) {
            return QImage();
        }

        QImage image(webpData, width, height, QImage::Format_ARGB32);
        QImage result = image.copy(); // 深拷贝，因为 webpData 将被释放

        WebPFree(webpData);
        return result;
    }

    // 处理动画 WebP（这里只取第一帧）
    int timestamp;
    uint8_t *frameData;

    if (!WebPAnimDecoderGetNext(dec, &frameData, &timestamp)) {
        WebPAnimDecoderDelete(dec);
        return QImage();
    }

    WebPAnimInfo anim_info;
    WebPAnimDecoderGetInfo(dec, &anim_info);

    QImage image(frameData, anim_info.canvas_width, anim_info.canvas_height, QImage::Format_ARGB32);
    QImage result = image.copy();

    WebPAnimDecoderDelete(dec);
    return result;
}
#endif

QImage ImageLoader::createQImageFromData(unsigned char *data, int width, int height, int channels) {
    if (!data || width <= 0 || height <= 0) {
        return QImage();
    }

    QImage::Format format;
    int bytesPerLine = 0;

    switch (channels) {
        case 1: // 灰度
            format = QImage::Format_Grayscale8;
            bytesPerLine = width;
            break;
        case 2: // 灰度 + alpha
            // Qt 没有直接的格式，转换为 ARGB32
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
        break;
        case 3: // RGB
            format = QImage::Format_RGB888;
            bytesPerLine = width * 3;
            // 需要重新排列，stb_image 返回的是 RGB
            {
                QImage image(width, height, QImage::Format_RGB888);
                for (int y = 0; y < height; ++y) {
                    memcpy(image.scanLine(y), data + y * width * 3, width * 3);
                }
                return image;
            }
            break;
        case 4: // RGBA
            format = QImage::Format_RGBA8888;
            bytesPerLine = width * 4;
            // stb_image 返回的是 RGBA，Qt 使用 ARGB，需要转换
            {
                QImage image(width, height, QImage::Format_ARGB32);
                for (int y = 0; y < height; ++y) {
                    QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
                    for (int x = 0; x < width; ++x) {
                        int offset = y * width * 4 + x * 4;
                        unsigned char r = data[offset];
                        unsigned char g = data[offset + 1];
                        unsigned char b = data[offset + 2];
                        unsigned char a = data[offset + 3];
                        scanLine[x] = qRgba(r, g, b, a);
                    }
                }
                return image;
            }
            break;
        default:
            qWarning() << "不支持的通道数:" << channels;
            return QImage();
    }

    return QImage(data, width, height, bytesPerLine, format);
}
