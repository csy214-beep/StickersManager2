#include "aboutpage.h"
#include "appinfo.h"
#include "configmanager.h"
#include "launcher.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QCoreApplication>
#include <QScrollArea>

QString AboutPage::formatSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024LL * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

int AboutPage::compareVersions(const QString &a, const QString &b) {
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

qint64 AboutPage::dirSize(const QString &path) const {
    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    qint64 total = 0;
    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

AboutPage::AboutPage(ConfigManager *config, QWidget *parent)
    : QWidget(parent), m_config(config)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *contentLayout = new QVBoxLayout(content);

    auto *infoGroup = new QGroupBox("Application");
    auto *infoForm = new QFormLayout(infoGroup);

    infoForm->addRow("Name:", new QLabel(AppInfo::name(), this));
    infoForm->addRow("Version:", new QLabel(AppInfo::version(), this));
    infoForm->addRow("Author:", new QLabel(AppInfo::author(), this));
    infoForm->addRow("License:", new QLabel(AppInfo::license(), this));

    auto *repoLink = new QLabel("<a href='#' style='color: palette(highlight);'>" + AppInfo::repoUrl() + "</a>", this);
    repoLink->setCursor(Qt::PointingHandCursor);
    connect(repoLink, &QLabel::linkActivated, []() { launch(AppInfo::repoUrl()); });
    infoForm->addRow("Repository:", repoLink);

    auto *issuesLink = new QLabel("<a href='#' style='color: palette(highlight);'>Feedback</a>", this);
    issuesLink->setCursor(Qt::PointingHandCursor);
    connect(issuesLink, &QLabel::linkActivated, []() { launch(AppInfo::issuesUrl()); });
    infoForm->addRow("Feedback:", issuesLink);

    contentLayout->addWidget(infoGroup);

    // Paths
    auto *pathsGroup = new QGroupBox("Paths");
    auto *pathsForm = new QFormLayout(pathsGroup);

    auto openLink = [](const QString &path) {
        launch(path);
    };

    QString appPath = QCoreApplication::applicationFilePath();
    auto *appPathLink = new QLabel("<a href='#' style='color: palette(highlight);'>" + appPath + "</a>", this);
    appPathLink->setCursor(Qt::PointingHandCursor);
    connect(appPathLink, &QLabel::linkActivated, this, [appPath]() { launch(QFileInfo(appPath).absolutePath()); });
    pathsForm->addRow("Executable:", appPathLink);

    QString cfgDir = QFileInfo(m_config->getConfigPath()).absolutePath();
    auto *cfgLink = new QLabel("<a href='#' style='color: palette(highlight);'>" + cfgDir + "</a>", this);
    cfgLink->setCursor(Qt::PointingHandCursor);
    connect(cfgLink, &QLabel::linkActivated, this, [cfgDir]() { launch(cfgDir); });
    pathsForm->addRow("Config folder:", cfgLink);

    contentLayout->addWidget(pathsGroup);

    // Storage
    auto *storageGroup = new QGroupBox("Storage");
    auto *storageForm = new QFormLayout(storageGroup);

    qint64 appDirBytes = dirSize(QCoreApplication::applicationDirPath());
    storageForm->addRow("App directory:", new QLabel(formatSize(appDirBytes), this));

    qint64 libBytes = 0;
    auto libs = m_config->getLibraries();
    for (const auto &lib : libs) {
        if (!lib.enabled || lib.path.isEmpty()) continue;
        libBytes += dirSize(lib.path);
    }
    m_storageLabel = new QLabel(formatSize(libBytes), this);
    storageForm->addRow("Library files:", m_storageLabel);

    contentLayout->addWidget(storageGroup);

    // Update
    auto *updateGroup = new QGroupBox("Update");
    auto *updateLayout = new QVBoxLayout(updateGroup);

    m_checkUpdateBtn = new QPushButton("Check for Updates", this);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);

    updateLayout->addWidget(m_checkUpdateBtn);
    updateLayout->addWidget(m_statusLabel);
    contentLayout->addWidget(updateGroup);
    contentLayout->addStretch();

    scrollArea->setWidget(content);
    root->addWidget(scrollArea);

    connect(m_checkUpdateBtn, &QPushButton::clicked, this, &AboutPage::checkForUpdates);
}

void AboutPage::checkForUpdates() {
    m_statusLabel->setText("Checking...");
    m_checkUpdateBtn->setEnabled(false);

    auto *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(AppInfo::apiReleasesUrl()));
    request.setRawHeader("User-Agent", "StickersManager");

    auto *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        reply->manager()->deleteLater();
        m_checkUpdateBtn->setEnabled(true);

        if (reply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText("Network error: " + reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            m_statusLabel->setText("Failed to parse response.");
            return;
        }

        QJsonObject obj = doc.object();
        QString latestVersion = obj["tag_name"].toString();
        if (latestVersion.isEmpty())
            latestVersion = obj["name"].toString();

        if (latestVersion.isEmpty()) {
            m_statusLabel->setText("Could not determine latest version.");
            return;
        }

        int cmp = compareVersions(latestVersion, AppInfo::version());
        if (cmp <= 0) {
            QMessageBox::information(this, "Up to Date",
                                     "You are running the latest version: " + AppInfo::version());
            m_statusLabel->setText("Latest: " + AppInfo::version());
        } else {
            auto result = QMessageBox::question(this, "Update Available",
                "Current version: " + AppInfo::version() + "\n"
                "Latest version: " + latestVersion + "\n\n"
                "Open release page?",
                QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes)
                launch(AppInfo::repoUrl() + "/releases/tag/" + latestVersion);
            m_statusLabel->setText("Latest: " + latestVersion);
        }
    });
}
