#include "Logger.h"
#include <QDateTime>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logFile("application.log"), logStream(&logFile) {
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        // 如果无法打开日志文件，可以抛出异常或采取其他处理措施
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
