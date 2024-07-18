#include "FileSearchThread.h"
#include "Logger.h"
#include <QMutexLocker>
#include <QDirIterator>
#include <QDialog>

FileSearchThread::FileSearchThread(const QString &keyword, const QString &path, QObject *parent)
        : QThread(parent), searchKeyword(keyword), searchPath(path), stopped(false) {}

void FileSearchThread::run() {
    QDirIterator it(searchPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        {
            QMutexLocker locker(&mutex);
            if (stopped) {
                Logger::instance().log("搜索线程被中断");
                break;
            }
        }

        QString filePath = it.next();
        if (filePath.contains(searchKeyword, Qt::CaseInsensitive)) {
            emit fileFound(filePath); // 发射 fileFound 信号
        }

        QThread::msleep(1); // 小休眠，减少CPU占用
    }
    emit searchFinished(); // 搜索完成后发射 searchFinished 信号
}

void FileSearchThread::stop() {
    QMutexLocker locker(&mutex);
    stopped = true;
}
