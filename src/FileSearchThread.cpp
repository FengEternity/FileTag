// FileSearchThread.cpp

#include "FileSearchThread.h"
#include "Logger.h"
#include <QDirIterator>
#include <QEventLoop>
#include <QMutexLocker>

FileSearchThread::FileSearchThread(const QString &keyword, QQueue<QString> *taskQueue, QMutex *queueMutex, QWaitCondition *queueCondition, QObject *parent)
        : QObject(parent), searchKeyword(keyword), taskQueue(taskQueue), queueMutex(queueMutex), queueCondition(queueCondition), stopped(false) {
    LOG_INFO("线程创建");
}

FileSearchThread::~FileSearchThread() {
    LOG_INFO("线程销毁");
}

void FileSearchThread::run() {
    while (true) {
        QString searchPath;
        {
            QMutexLocker locker(queueMutex);
            if (taskQueue->isEmpty()) {
                if (stopped) {
                    break;
                }
                queueCondition->wait(queueMutex);
                continue;
            }
            searchPath = taskQueue->dequeue();

            emit taskStarted();
        }

        if (searchPath.isEmpty()) {
            continue;
        }

        LOG_INFO("线程开始：" + searchPath);
        QDirIterator it(searchPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        QEventLoop loop;
        while (it.hasNext() && !stopped) {
            QString filePath = it.next();
            QString fileName = it.fileName();

            if (fileName.contains(searchKeyword, Qt::CaseInsensitive)) {
                emit fileFound(filePath);
            }

            loop.processEvents(QEventLoop::AllEvents, 50);
        }
        emit searchFinished();
        LOG_INFO("线程结束：" + searchPath);
    }
}

void FileSearchThread::stop() {
    stopped = true;
}
