#ifndef LOGGER_H
#define LOGGER_H
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger {
public:
    enum Level { Debug, Info, Warning, Error };

    static void init();
    static void log(Level level, const QString& message);

    // 便捷接口
    static void debug(const QString& msg) { log(Debug, msg); }
    static void info(const QString& msg)  { log(Info, msg); }
    static void warn(const QString& msg)  { log(Warning, msg); }
    static void error(const QString& msg) { log(Error, msg); }

private:
    static QFile* s_logFile;
    static QTextStream* s_logStream;
    static QMutex s_mutex;

    static QString logFilePath();
    static QString levelToString(Level level);
};
#endif // LOGGER_H
