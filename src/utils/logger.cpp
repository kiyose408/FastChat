
#include "logger.h"
#include <QStandardPaths>
#include <QDir>
#include <iostream>

QFile* Logger::s_logFile = nullptr;
QTextStream* Logger::s_logStream = nullptr;
QMutex Logger::s_mutex;

QString Logger::logFilePath() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
    + "/logs";
    QDir().mkpath(dir);
    return dir + "/fastchat_" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
}

void Logger::init() {
    s_logFile = new QFile(logFilePath());
    if (s_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        s_logStream = new QTextStream(s_logFile);
    }

    // 安装全局消息处理器（捕获 qDebug() 等）
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext&, const QString& msg) {
        Level level = Info;
        switch (type) {
        case QtDebugMsg:    level = Debug; break;
        case QtInfoMsg:     level = Info; break;
        case QtWarningMsg:  level = Warning; break;
        case QtCriticalMsg:
        case QtFatalMsg:    level = Error; break;
        }
        Logger::log(level, msg);
    });
}

void Logger::log(Level level, const QString& message) {
    QMutexLocker locker(&s_mutex);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logLine = QString("[%1] [%2] %3")
                          .arg(timestamp)
                          .arg(levelToString(level))
                          .arg(message);

    // 输出到文件
    if (s_logStream) {
        *s_logStream << logLine << "\n";
        s_logStream->flush(); // 确保立即写入
    }

    // 同时输出到控制台（调试时有用）
    std::cout << logLine.toStdString() << std::endl;
}

QString Logger::levelToString(Level level) {
    switch (level) {
    case Debug:   return "DEBUG";
    case Info:    return "INFO";
    case Warning: return "WARN";
    case Error:   return "ERROR";
    }
    return "UNKNOWN";
}
