#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    struct Result {
        bool success = false;
        QString latestVersion;
        QString error;
    };

    explicit UpdateChecker(QObject *parent = nullptr);

    static int compareVersions(const QString &a, const QString &b);

    static Result lastResult;

    void check();

signals:
    void finished(bool success, const QString &latestVersion, const QString &error);

private:
    QNetworkAccessManager *m_manager;
};

#endif // UPDATECHECKER_H
