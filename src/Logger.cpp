#include "Logger.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QMutexLocker>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logFile(generateLogFileName()), logStream(&logFile), running(true) {
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

void Logger::log(const QString &message) {
    {
        QMutexLocker locker(&mutex);
        logQueue.enqueue(message);  // 将消息放入队列
    }
    condition.wakeOne();  // 唤醒工作线程
}

void Logger::run() {
    while (running) {
        QString message;

        {
            QMutexLocker locker(&mutex);
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
    QString dateString = QDate::currentDate().toString("yyyyMMdd");
    return QString("logs/application_%1.log").arg(dateString);
}
