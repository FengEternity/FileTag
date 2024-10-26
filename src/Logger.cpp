#include "Logger.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QMutexLocker>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
        : logFile(generateLogFileName()),
          logStream(&logFile),
          running(true),
          currentLogLevel(LogLevel::INFO),
          identifier("") {
    QDir logDir("logs");
    if (!logDir.exists()) {
        qDebug() << "日志目录不存在，尝试创建";
        if (logDir.mkpath(".")) {
            qDebug() << "日志目录创建成功";
        } else {
            qDebug() << "日志目录创建失败";
        }
    }

    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream(stderr) << "无法打开日志文件: " << logFile.fileName() << "\n";
        qDebug() << "无法打开日志文件: " << logFile.fileName();
    } else {
        qDebug() << "日志文件打开成功: " << logFile.fileName();
    }

    start();  // 启动线程
}

Logger::~Logger() {
    {
        QMutexLocker locker(&mutex);
        running = false;  // 设置线程运行状态为 false
    }
    condition.wakeOne();  // 唤醒线程以便它可以退出
    wait();  // 等待线程结束
    logFile.close();
}

void Logger::setLogLevel(LogLevel level) {
    QMutexLocker locker(&mutex);
    currentLogLevel = level;
}

void Logger::log(const QString &message, LogLevel level, const char* file, int line, const char* function) {

//    if (level < currentLogLevel) {
//        return;  // 如果消息级别低于当前日志级别，则忽略该消息
//    }

    QString threadId = getCurrentThreadId();
    QString sourceInfo = QString("[%1:%2 - %3]").arg(file).arg(line).arg(function);

    QString currentIdentifier;
    {
        QMutexLocker locker(&mutex);
        currentIdentifier = identifier;
    }

    QString identifierPart = currentIdentifier.isEmpty() ? "" : QString("[标识符: %1] ").arg(currentIdentifier);

    QString logMessage = QString("[%1] [线程ID: %2] %3%4 %5")
            .arg(logLevelToString(level, false))  // 文件日志不带颜色
            .arg(threadId)
            .arg(identifierPart)  // 标识符部分
            .arg(message)
            .arg(sourceInfo);

    {
        QMutexLocker locker(&mutex);
        logQueue.enqueue(logMessage);
    }
    condition.wakeOne();  // 唤醒工作线程

    // 控制台输出带颜色的版本
    QString consoleMessage = QString("[%1] [线程ID: %2] %3%4 %5")
            .arg(logLevelToString(level, true))  // 控制台输出带颜色
            .arg(threadId)
            .arg(identifierPart)  // 标识符部分
            .arg(message)
            .arg(sourceInfo);
    QTextStream(stdout) << consoleMessage << "\n";  // 输出到标准输出
}

void Logger::run() {
    while (true) {
        QString message;

        {
            QMutexLocker locker(&mutex);
            if (logQueue.isEmpty() && !running) {
                break;  // 如果队列为空且不再运行，则退出线程
            }
            if (logQueue.isEmpty()) {
                condition.wait(&mutex);  // 如果队列为空，等待唤醒
            }
            if (!logQueue.isEmpty()) {
                message = logQueue.dequeue();  // 从队列中取出消息
            }
        }

        if (!message.isEmpty()) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            if (!logFile.isOpen()) {
                logFile.setFileName(generateLogFileName());
                logFile.open(QIODevice::Append | QIODevice::Text);
                logStream.setDevice(&logFile);
            }
            logStream << timestamp << " - " << message << "\n";
            logStream.flush();
        }
    }
}

void Logger::rotateLogFile() {
    if (logFile.isOpen()) {
        logFile.close();
    }

    logFile.setFileName(generateLogFileName());
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream(stderr) << "无法打开新的日志文件: " << logFile.fileName() << "\n";
        qDebug() << "无法打开新的日志文件: " << logFile.fileName();
    } else {
        logStream.setDevice(&logFile);
        qDebug() << "新的日志文件打开成功: " << logFile.fileName();
    }
}

QString Logger::generateLogFileName() {
    QString dateTimeString = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    return QString("logs/application_%1.log").arg(dateTimeString);
}

QString Logger::logLevelToString(LogLevel level, bool useColor) {
    if (useColor) {
        switch (level) {
            case LogLevel::DEBUG: return "\033[36mDEBUG\033[0m";   // 青色
            case LogLevel::INFO: return "\033[37mINFO\033[0m";    // 白色
            case LogLevel::WARNING: return "\033[33mWARNING\033[0m"; // 黄色
            case LogLevel::ERROR: return "\033[31mERROR\033[0m";  // 红色
            default: return "\033[37mUNKNOWN\033[0m";             // 默认白色
        }
    } else {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
}

QString Logger::getCurrentThreadId() {
    return QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16);
}
