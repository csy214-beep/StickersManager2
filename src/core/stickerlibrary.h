#ifndef STICKERLIBRARY_H
#define STICKERLIBRARY_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QHash>
#include <QDir>
#include <QFileInfo>
#include <QSet>

class StickerLibrary : public QObject {
    Q_OBJECT

public:
    explicit StickerLibrary(QObject *parent = nullptr);

    bool setLibraryPath(const QString &path);
    bool scanLibrary();

    QMap<QString, QVector<QString> > getCategories() const { return m_categories; }

    QVector<QString> searchStickers(const QString &keyword);

private:
    QString m_libraryPath;
    QMap<QString, QVector<QString> > m_categories;
    QVector<QString> m_allStickers;
    QHash<QString, QString> m_searchIndex;

    bool isValidImage(const QString &filePath);
};

#endif // STICKERLIBRARY_H
