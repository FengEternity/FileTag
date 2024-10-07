#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDate>

class Logger {
public:
    static Logger& instance();
    void log(const QString &message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void rotateLogFiles();
    QString generateLogFileName();

    QFile logFile;
    QTextStream logStream;
    QMutex mutex;
    QDate currentDate;
};

#endif // LOGGER_H
