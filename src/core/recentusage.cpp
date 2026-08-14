#include "recentusage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QCryptographicHash>
#include <QDateTime>
#include <algorithm>

void RecentUsageStore::load(const QString &libraryPath) {
    m_entries.clear();
    m_filePath.clear();

    if (libraryPath.isEmpty())
        return;

    QString hash = QCryptographicHash::hash(libraryPath.toUtf8(), QCryptographicHash::Md5).toHex();
    QString configDir = QCoreApplication::applicationDirPath() + "/.stickersmanager";
    m_filePath = configDir + "/recent_" + hash + ".json";

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject())
        return;

    QJsonArray entries = doc.object()["entries"].toArray();
    for (const QJsonValue &val : entries) {
        QJsonObject obj = val.toObject();
        Entry entry;
        entry.path = obj["path"].toString();
        entry.time = obj["time"].toVariant().toLongLong();
        if (entry.path.isEmpty())
            continue;
        m_entries.append(entry);
    }

    std::sort(m_entries.begin(), m_entries.end(),
              [](const Entry &a, const Entry &b) { return a.time > b.time; });

    m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                                   [](const Entry &e) { return !QFile::exists(e.path); }),
                    m_entries.end());

    if (m_entries.size() > kMaxEntries)
        m_entries.resize(kMaxEntries);
}

void RecentUsageStore::recordUse(const QString &filePath) {
    if (filePath.isEmpty() || m_filePath.isEmpty())
        return;

    QString canonical = QFileInfo(filePath).canonicalFilePath();
    if (canonical.isEmpty())
        return;

    m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                                   [&canonical](const Entry &e) {
                                       return QFileInfo(e.path).canonicalFilePath() == canonical;
                                   }),
                    m_entries.end());

    Entry entry;
    entry.path = filePath;
    entry.time = QDateTime::currentMSecsSinceEpoch();
    m_entries.prepend(entry);

    if (m_entries.size() > kMaxEntries)
        m_entries.resize(kMaxEntries);

    save();
}

void RecentUsageStore::clear() {
    m_entries.clear();
    if (!m_filePath.isEmpty())
        QFile::remove(m_filePath);
}

QVector<QString> RecentUsageStore::paths() const {
    QVector<QString> result;
    result.reserve(qMin(m_entries.size(), m_limit));
    int shown = 0;
    for (const Entry &e : m_entries) {
        if (shown >= m_limit)
            break;
        if (QFile::exists(e.path)) {
            result.append(e.path);
            ++shown;
        }
    }
    return result;
}

bool RecentUsageStore::save() {
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());

    QJsonArray entries;
    for (const Entry &e : m_entries) {
        QJsonObject obj;
        obj["path"] = e.path;
        obj["time"] = e.time;
        entries.append(obj);
    }

    QJsonObject root;
    root["entries"] = entries;

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(QJsonDocument(root).toJson()) == -1 || !file.commit())
        return false;
    return true;
}