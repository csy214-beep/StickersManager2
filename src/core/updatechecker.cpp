#include "updatechecker.h"
#include "appinfo.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QVector>

UpdateChecker::Result UpdateChecker::lastResult;

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this))
{
}

int UpdateChecker::compareVersions(const QString &a, const QString &b) {
    auto parts = [](const QString &v) {
        QString s = v;
        if (s.startsWith('v')) s = s.mid(1);
        QStringList p = s.split('.');
        return QVector<int>{p.size() > 0 ? p[0].toInt() : 0,
                           p.size() > 1 ? p[1].toInt() : 0,
                           p.size() > 2 ? p[2].toInt() : 0};
    };
    QVector<int> va = parts(a), vb = parts(b);
    for (int i = 0; i < 3; ++i) {
        if (va[i] < vb[i]) return -1;
        if (va[i] > vb[i]) return 1;
    }
    return 0;
}

void UpdateChecker::check() {
    QNetworkRequest request{QUrl(AppInfo::apiReleasesUrl())};
    request.setRawHeader("User-Agent", "StickersManager");

    auto *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            lastResult = {false, QString(), reply->errorString()};
            emit finished(false, QString(), reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            lastResult = {false, QString(), "Failed to parse response."};
            emit finished(false, QString(), "Failed to parse response.");
            return;
        }

        QJsonObject obj = doc.object();
        QString latestVersion = obj["tag_name"].toString();
        if (latestVersion.isEmpty())
            latestVersion = obj["name"].toString();

        if (latestVersion.isEmpty()) {
            lastResult = {false, QString(), "Could not determine latest version."};
            emit finished(false, QString(), "Could not determine latest version.");
            return;
        }

        lastResult = {true, latestVersion, QString()};
        emit finished(true, latestVersion, QString());
    });
}
