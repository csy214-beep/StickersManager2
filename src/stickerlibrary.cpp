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
        emit errorOccurred("目录不存在: " + path);
        return false;
    }

    m_libraryPath = path;
    return scanLibrary();
}

bool StickerLibrary::scanLibrary() {
    if (m_libraryPath.isEmpty()) {
        emit errorOccurred("未设置表情库路径");
        return false;
    }

    m_categories.clear();
    m_allStickers.clear();

    QDir libraryDir(m_libraryPath);
    if (!libraryDir.exists()) {
        emit errorOccurred("表情库目录不存在: " + m_libraryPath);
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
            qDebug() << "分类" << categoryName << "加载了" << imageFiles.size() << "个表情";
        }
    }

    qDebug() << "表情库扫描完成，共" << m_categories.size() << "个分类，"
            << totalStickers << "个表情";
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
