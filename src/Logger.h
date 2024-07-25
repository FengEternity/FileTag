#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger {
public:
    static Logger& instance();
    void log(const QString &message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QFile logFile;
    QTextStream logStream;
    QMutex mutex;
};

#endif // LOGGER_H
