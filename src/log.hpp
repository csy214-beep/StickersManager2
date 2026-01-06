#pragma once
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <QString>
#include <QDir>
#define QT_LOG_FILE "log/log.log"
// 输出到控制台（如果启用）
#ifdef CONSOLE
QTextStream out(stdout);
out<< txt<< Qt::endl;
#endif

// 初始化日志文件
static void initLogFile() {
    // 如果目录不存在则创建，包括所有必要的父目录
    QDir().mkpath("log");

    QFileInfo fileInfo(QT_LOG_FILE);
    if (fileInfo.exists()) {
        // 如果日志文件存在，则删除
        QFile::remove(QT_LOG_FILE);
    }
}


// 自定义消息处理函数
inline void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context)
    QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("[%1] Debug: %2").arg(logTime).arg(msg);
            break;
        case QtInfoMsg:
            txt = QString("[%1] Info: %2").arg(logTime).arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("[%1] Warning: %2").arg(logTime).arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("[%1] Critical: %2").arg(logTime).arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("[%1] Fatal: %2").arg(logTime).arg(msg);
            break;
    }

    // 输出到日志文件
    QFile logFile(QT_LOG_FILE);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream textStream(&logFile);
        textStream.setEncoding(QStringConverter::Utf8);
        textStream << txt << Qt::endl;
        logFile.close();
    }
}
