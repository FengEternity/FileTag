#include "Logger.h"
#include <QDateTime>
#include <QDir>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logFile("logs/application.log"), logStream(&logFile) {
    QDir logDir("logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        // 如果无法打开日志文件，可以输出到标准错误流
        QTextStream(stderr) << "无法打开日志文件: " << logFile.fileName() << "\n";
    }
}

Logger::~Logger() {
    logFile.close();
}

void Logger::log(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    logStream << timestamp << " - " << message << "\n";
    logStream.flush();
}
