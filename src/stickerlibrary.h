#ifndef STICKERLIBRARY_H
#define STICKERLIBRARY_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QDir>
#include <QFileInfo>
#include <QSet>

class StickerLibrary : public QObject {
    Q_OBJECT

public:
    explicit StickerLibrary(QObject *parent = nullptr);

    // 库操作
    bool setLibraryPath(const QString &path);

    QString getLibraryPath() const { return m_libraryPath; }

    bool scanLibrary();


    // 数据访问
    QMap<QString, QVector<QString> > getCategories() const { return m_categories; }
    QVector<QString> getAllStickers() const { return m_allStickers; }

    // 搜索
    QVector<QString> searchStickers(const QString &keyword);

    // 支持的格式
    static QSet<QString> getSupportedFormats();

signals:
    void libraryLoaded(bool success);

    void errorOccurred(const QString &error);

private:
    QString m_libraryPath;
    QMap<QString, QVector<QString> > m_categories; // 分类名 -> 表情路径列表
    QVector<QString> m_allStickers;

    bool isValidImage(const QString &filePath);
};

#endif // STICKERLIBRARY_H
