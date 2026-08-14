#ifndef RECENTUSAGE_H
#define RECENTUSAGE_H

#include <QString>
#include <QVector>

class RecentUsageStore {
public:
    struct Entry {
        QString path;
        qint64 time;
    };

    void load(const QString &libraryPath);
    void recordUse(const QString &filePath);
    void clear();
    void setLimit(int limit) { m_limit = qMax(1, limit); }
    QVector<QString> paths() const;

private:
    bool save();

    QString m_filePath;
    QVector<Entry> m_entries;
    int m_limit = 100;
    static constexpr int kMaxEntries = 100;
};

#endif // RECENTUSAGE_H