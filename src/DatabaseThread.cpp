//
// Created by Monty-Lee  on 24-10-31.
//

#include "DatabaseThread.h"
#include "FileIndexDatabase.h"
#include <QDebug>

DatabaseThread::DatabaseThread(AbstractDatabase *db, QObject *parent)
        : QThread(parent), db(db), isRunning(true) {
    start();
}

DatabaseThread::~DatabaseThread() {
    // 请求线程停止并等待其结束
    {
        QMutexLocker locker(&mutex);
        isRunning = false;
        condition.wakeAll();
    }
    wait();
}

void DatabaseThread::addInsertFileTask(const QString &filePath) {
    QMutexLocker locker(&mutex);
    taskQueue.enqueue({ Task::InsertFile, filePath });
    condition.wakeOne();
}

void DatabaseThread::addSearchFilesTask(const QString &keyword) {
    QMutexLocker locker(&mutex);
    taskQueue.enqueue({ Task::SearchFiles, keyword });
    condition.wakeOne();
}

void DatabaseThread::run() {
    while (true) {
        Task task;
        {
            QMutexLocker locker(&mutex);
            while (taskQueue.isEmpty() && isRunning) {
                condition.wait(&mutex);
            }
            if (!isRunning && taskQueue.isEmpty()) {
                break;
            }
            task = taskQueue.dequeue();
        }

        switch (task.type) {
            case Task::InsertFile:
                processInsertFile(task.data.toString());
                break;
            case Task::SearchFiles:
                processSearchFiles(task.data.toString());
                break;
        }
    }
}

void DatabaseThread::processInsertFile(const QString &filePath) {
    // 调用数据库的插入文件信息方法
    if (auto fileDb = dynamic_cast<FileIndexDatabase*>(db)) {
        if (fileDb->insertFileInfo(filePath)) {
            emit fileInserted(filePath);
        } else {
            qDebug() << "插入文件信息失败：" << filePath;
        }
    }
}

void DatabaseThread::processSearchFiles(const QString &keyword) {
    // 调用数据库的搜索文件方法
    if (auto fileDb = dynamic_cast<FileIndexDatabase*>(db)) {
        QVector<QString> results = fileDb->searchFiles(keyword);
        emit searchFinished(results);
    }
}
