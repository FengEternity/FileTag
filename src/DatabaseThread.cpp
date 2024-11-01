//
// Created by Monty-Lee  on 24-10-31.
//

#include "DatabaseThread.h"
#include "Logger.h"

DatabaseThread::DatabaseThread(FileDatabase *db, QObject *parent) : QThread(parent), db(db) {
    LOG_INFO("数据库线程创建成功。");
}

DatabaseThread::~DatabaseThread() {
    LOG_INFO("数据库线程销毁。");
}

void DatabaseThread::run() {
//    if (!db->openDatabase()) {
//        LOG_ERROR("数据库打开失败。");
//        return;
//    }

    if (!filePath.isEmpty()) {
        db->insertFileInfo(filePath);
        emit fileInserted(filePath);
    } else {
        QVector<QString> result = db->searchFiles(keyword);
        emit searchFinished(result);
    }

    // db->closeDatabase();
}


void DatabaseThread::insertFileInfo(const QString &filePath) {
    this->filePath = filePath;
    start();
}

void DatabaseThread::searchFiles(const QString &keyword) {
    this->keyword = keyword;
    start();
}