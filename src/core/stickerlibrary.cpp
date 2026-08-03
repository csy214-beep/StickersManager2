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
        return false;
    }

    m_libraryPath = path;
    return scanLibrary();
}

bool StickerLibrary::scanLibrary() {
    if (m_libraryPath.isEmpty()) {
        return false;
    }

    m_categories.clear();
    m_allStickers.clear();
    m_searchIndex.clear();

    QDir libraryDir(m_libraryPath);
    if (!libraryDir.exists()) {
        return false;
    }

    QStringList categoryDirs = libraryDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    int totalStickers = 0;
    for (const QString &categoryName: categoryDirs) {
        QString categoryPath = m_libraryPath + "/" + categoryName;
        QDir categoryDir(categoryPath);

        QStringList imageFiles;
        QStringList files = categoryDir.entryList(QDir::Files);
        for (const QString &fileName: files) {
            if (fileName.startsWith(".preview")) {
                continue;
            }

            if (fileName.contains(".preview.")) {
                continue;
            }

            QString filePath = categoryPath + "/" + fileName;
            if (isValidImage(filePath)) {
                imageFiles.append(filePath);
            }
        }

        if (!imageFiles.isEmpty()) {
            std::sort(imageFiles.begin(), imageFiles.end());
            m_categories[categoryName] = QVector<QString>::fromList(imageFiles);
            for (const QString &path : imageFiles)
                m_searchIndex.insert(path, QFileInfo(path).fileName().toLower());
            m_allStickers.append(imageFiles);
            totalStickers += imageFiles.size();
            qDebug() << "Category" << categoryName << "loaded" << imageFiles.size() << "stickers";
        }
    }

    qDebug() << "Sticker library scan complete, total" << m_categories.size() << "categories,"
             << totalStickers << "stickers";
    return true;
}

QVector<QString> StickerLibrary::searchStickers(const QString &keyword) {
    if (keyword.isEmpty()) {
        return QVector<QString>();
    }
    QString lowerKeyword = keyword.toLower();
    QVector<QString> results;

    for (const QString &stickerPath: m_allStickers) {
        if (m_searchIndex.value(stickerPath).contains(lowerKeyword)) {
            results.append(stickerPath);
        }
    }

    return results;
}

bool StickerLibrary::isValidImage(const QString &filePath) {
    return ImageLoader::isFormatSupported(filePath);
}
