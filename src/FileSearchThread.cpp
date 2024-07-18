#include "FileSearchThread.h"
#include <QMutexLocker>
#include <QDirIterator>
#include <QCoreApplication>
#include <QEventLoop>
#include "Logger.h" // 引入 Logger 头文件

FileSearchThread::FileSearchThread(const QString &keyword, const QString &path, QObject *parent)
        : QThread(parent), searchKeyword(keyword), searchPath(path), stopped(false) {}

void FileSearchThread::run() {
    timer.start(); // 开始计时
    QDirIterator it(searchPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QEventLoop loop;
    while (it.hasNext()) {
        {
            QMutexLocker locker(&mutex);
            if (stopped) {
                Logger::instance().log("搜索线程被中断");
                break;
            }
        }

        QString filePath = it.next();
        QString fileName = it.fileName();

        if (fileName.contains(searchKeyword, Qt::CaseInsensitive)) {
            emit fileFound(filePath); // 发射 fileFound 信号
        }

        loop.processEvents(QEventLoop::AllEvents, static_cast<int>(10));
    }
    emit searchFinished(); // 搜索完成后发射 searchFinished 信号
    emit searchTime(timer.elapsed()); // 发射 searchTime 信号，传递耗时
}

void FileSearchThread::stop() {
    QMutexLocker locker(&mutex);
    stopped = true;
}
