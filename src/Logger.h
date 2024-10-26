#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>
#include <atomic>

// 辅助宏，用于简化调用，包含文件名、行号和函数名
#define LOG_ERROR(message) \
    Logger::instance().log(message, LogLevel::ERROR, __FILE__, __LINE__, __FUNCTION__)

#define LOG_WARNING(message) \
    Logger::instance().log(message, LogLevel::WARNING, __FILE__, __LINE__, __FUNCTION__)

#define LOG_INFO(message) \
    Logger::instance().log(message, LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__)

#define LOG_DEBUG(message) \
    Logger::instance().log(message, LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__)

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger : public QThread {
Q_OBJECT

public:
    static Logger& instance();
    void log(const QString &message, LogLevel level, const char* file, int line, const char* function);
    void setLogLevel(LogLevel level);
    void rotateLogFile();

protected:
    void run() override;  // 线程运行方法

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QString getCurrentThreadId();
    QString generateLogFileName();
    QString logLevelToString(LogLevel level, bool useColor = false);

    QFile logFile;
    QTextStream logStream;
    QMutex mutex;
    QQueue<QString> logQueue;  // 存储待写入的日志消息
    QWaitCondition condition;  // 等待条件，用于控制线程
    std::atomic<bool> running; // 控制线程运行状态

    LogLevel currentLogLevel; // 当前日志级别
    QString identifier;       // 标识符
    void setIdentifier(const QString &id);
};

#endif // LOGGER_H
