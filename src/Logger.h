#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>

class Logger : public QThread {
    Q_OBJECT

public:
    static Logger& instance();
    void log(const QString &message);

protected:
    void run() override;  // 线程运行方法

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void rotateLogFile();
    QString generateLogFileName();

    QFile logFile;
    QTextStream logStream;
    QMutex mutex;
    QQueue<QString> logQueue;  // 存储待写入的日志消息
    QWaitCondition condition;    // 等待条件，用于控制线程
    bool running;               // 控制线程运行状态
};

#endif // LOGGER_H
