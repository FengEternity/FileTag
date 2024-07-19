#include "FileSearchThread.h"
#include <QEventLoop>
#include <QDebug>
#include "Logger.h"

FileSearchThread::FileSearchThread(const QString &keyword, const QString &path, QObject *parent)
        : QObject(parent), searchKeyword(keyword), searchPath(path) {
    Logger::instance().log("线程创建："+searchPath);
}

FileSearchThread::~FileSearchThread() {
    Logger::instance().log("线程销毁：" + searchPath);
}

void FileSearchThread::run() {
    Logger::instance().log("线程开始："+searchPath);
    timer.start(); // 开始计时
    Logger::instance().log("开始计时："+searchPath);
    QDirIterator it(searchPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QEventLoop loop;
    while (it.hasNext()) {
        QString filePath = it.next();
        QString fileName = it.fileName();

        if (fileName.contains(searchKeyword, Qt::CaseInsensitive)) {
            emit fileFound(filePath); // 发射 fileFound 信号
        }

        // 增加事件处理间隔时间
        loop.processEvents(QEventLoop::AllEvents, static_cast<int>(50));
    }
    emit searchFinished(); // 搜索完成后发射 searchFinished 信号
    Logger::instance().log("线程结束："+searchPath);
    emit searchTime(timer.elapsed()); // 发射 searchTime 信号，传递耗时
}
