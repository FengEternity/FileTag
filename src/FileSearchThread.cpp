#include "FileSearchThread.h"
#include "Logger.h"
#include <QDirIterator>
#include <QEventLoop>
#include <QMutexLocker>

FileSearchThread::FileSearchThread(const QString &keyword, QQueue<QString> *taskQueue, QMutex *queueMutex, QWaitCondition *queueCondition, QObject *parent)
        : QObject(parent), searchKeyword(keyword), taskQueue(taskQueue), queueMutex(queueMutex), queueCondition(queueCondition), stopped(false) {
    Logger::instance().log("线程创建");
}

FileSearchThread::~FileSearchThread() {
    Logger::instance().log("线程销毁");
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
        }

        if (searchPath.isEmpty()) {
            continue;
        }

        Logger::instance().log("线程开始：" + searchPath);
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
        Logger::instance().log("线程结束：" + searchPath);
    }
}

void FileSearchThread::stop() {
    stopped = true;
}
