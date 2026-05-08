#include "stickerlibrary.h"
#include "imageloader.h"
#include <QDebug>
#include <QFileInfo>

StickerLibrary::StickerLibrary(QObject *parent)
    : QObject(parent) {
}

bool StickerLibrary::setLibraryPath(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        emit errorOccurred("Directory does not exist: " + path);
        return false;
    }

    m_libraryPath = path;
    return scanLibrary();
}

bool StickerLibrary::scanLibrary() {
    if (m_libraryPath.isEmpty()) {
        emit errorOccurred("Sticker library path not set");
        return false;
    }

    m_categories.clear();
    m_allStickers.clear();

    QDir libraryDir(m_libraryPath);
    if (!libraryDir.exists()) {
        emit errorOccurred("Sticker library directory does not exist: " + m_libraryPath);
        return false;
    }

    // 获取所有子目录（作为分类）
    QStringList categoryDirs = libraryDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    int totalStickers = 0;
    for (const QString &categoryName: categoryDirs) {
        QString categoryPath = m_libraryPath + "/" + categoryName;
        QDir categoryDir(categoryPath);

        // 获取分类下的所有图片文件
        QStringList imageFiles;
        QStringList files = categoryDir.entryList(QDir::Files);
        for (const QString &fileName: files) {
            // 跳过预览图文件 - 修复这里的逻辑
            if (fileName.startsWith(".preview")) {
                continue;
            }

            // 或者使用正则表达式匹配 .preview.*
            if (fileName.contains(".preview.")) {
                continue;
            }

            QString filePath = categoryPath + "/" + fileName;
            if (isValidImage(filePath)) {
                imageFiles.append(filePath);
            }
        }

        if (!imageFiles.isEmpty()) {
            // 按文件名排序
            std::sort(imageFiles.begin(), imageFiles.end());
            m_categories[categoryName] = QVector<QString>::fromList(imageFiles);
            m_allStickers.append(imageFiles);
            totalStickers += imageFiles.size();
            qDebug() << "Category" << categoryName << "loaded" << imageFiles.size() << "stickers";
        }
    }

    qDebug() << "Sticker library scan complete, total" << m_categories.size() << "categories,"
             << totalStickers << "stickers";
    emit libraryLoaded(true);
    return true;
}

QVector<QString> StickerLibrary::searchStickers(const QString &keyword) {
    if (keyword.isEmpty()) {
        return QVector<QString>();
    }
    QString lowerKeyword = keyword.toLower();
    QVector<QString> results;

    for (const QString &stickerPath: m_allStickers) {
        QFileInfo fileInfo(stickerPath);
        if (fileInfo.fileName().toLower().contains(lowerKeyword)) {
            results.append(stickerPath);
        }
    }

    return results;
}

bool StickerLibrary::isValidImage(const QString &filePath) {
    // 使用 ImageLoader 检查是否支持该格式
    return ImageLoader::isFormatSupported(filePath);
}

QSet<QString> StickerLibrary::getSupportedFormats() {
    // 获取 ImageLoader 支持的所有扩展名
    QStringList extensions = ImageLoader::getSupportedExtensions();
    return QSet<QString>(extensions.begin(), extensions.end());
}
