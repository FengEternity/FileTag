#include "Logger.h"
#include <QDateTime>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logFile("logs/application.log"), logStream(&logFile) {
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
        // 如果无法打开日志文件，可以输出到标准错误流
        QTextStream(stderr) << "无法打开日志文件: " << logFile.fileName() << "\n";
        qDebug() << "无法打开日志文件: " << logFile.fileName();
    } else {
        qDebug() << "日志文件打开成功: " << logFile.fileName();
    }
}

Logger::~Logger() {
    logFile.close();
}

void Logger::log(const QString &message) {
    QMutexLocker locker(&mutex);  // 加锁，确保多线程环境下的安全
    if (!logFile.isOpen()) {
        qDebug() << "日志文件未打开!";
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    logStream << timestamp << " - " << message << "\n";
    logStream.flush();

    if (logStream.status() != QTextStream::Ok) {
        qDebug() << "日志写入失败!";
    } else {
        qDebug() << "日志写入成功: " << timestamp << " - " << message;
    }
}
