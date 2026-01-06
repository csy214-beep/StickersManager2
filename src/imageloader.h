#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QImage>
#include <QString>
#include <functional>

class ImageLoader {
public:
    // 支持的格式
    enum class Format {
        Unknown,
        PNG, JPEG, GIF, BMP, TIFF, WEBP,
        PSD, HDR, TGA, // stb_image 支持的格式
        ICO, SVG, HEIC, AVIF // Qt 或其他库支持的格式
    };

    // 加载图像
    static QImage loadImage(const QString &filePath);

    // 检查是否支持该格式
    static bool isFormatSupported(const QString &filePath);

    // 获取所有支持的格式扩展名
    static QStringList getSupportedExtensions();

    // 获取格式描述
    static QString getFormatDescription(Format format);

    // 设置图像加载回调（用于进度显示等）
    static void setProgressCallback(std::function<void(int)> callback);

private:
    // 使用 stb_image 加载
    static QImage loadWithStb(const QString &filePath);

    // 使用 Qt 加载（备用）
    static QImage loadWithQt(const QString &filePath);

    // 使用特定库加载特殊格式
    static QImage loadWebP(const QString &filePath);

    // 检测文件格式
    static Format detectFormat(const QString &filePath);

    // 将 RGB/RGBA 数据转换为 QImage
    static QImage createQImageFromData(unsigned char *data, int width, int height, int channels);

    static std::function<void(int)> progressCallback;
};

#endif // IMAGELOADER_H
