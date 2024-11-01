/*
 * FileSearchCore.cpp
 * Author: Montee
 * CreateDate: 2024-11-1
 * Updater: Montee
 * UpdateDate: 2024-11-1
 * Summary: 文件文件搜索核心逻辑
 */

#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>
#include <QMetaObject>

#include "Logger.h"
#include "FileSearchCore.h"
// 初始化静态成员变量
QVector<QString> FileSearchCore::filesBatch;

/*
 * Summary: 构造函数，初始化成员变量和数据库连接
 * Parameters:
 * QObject *parent - 父对象指针，默认值为 nullptr
 * Return: 无
 */
FileSearchCore::FileSearchCore(QObject *parent)
        : QObject(parent),
          threadPool(new QThreadPool(this)),
          updateCounter(0),
          activeTaskCount(0),
          totalDirectories(0),
          isSearching(false),
          firstSearch(true),
          isStopping(false),
          db(new FileDatabase("file_index.db")),
          dbThread(new DatabaseThread(db, this)),
          taskQueue(new QQueue<QString>()),
          queueMutex(new QMutex()),
          queueCondition(new QWaitCondition())
{
    threadPool->setMaxThreadCount(QThread::idealThreadCount());

    // 初始化数据库并创建表
    if (db->openDatabase()) {
        if (!db->createTables()) {
            LOG_ERROR("数据库表创建失败。");
        }
    } else {
        LOG_ERROR("数据库打开失败。");
    }

    connect(dbThread, &DatabaseThread::fileInserted, this, &FileSearchCore::onFileInserted);
}

/*
 * Summary: 析构函数，释放资源
 * Parameters: 无
 * Return: 无
 */
FileSearchCore::~FileSearchCore() {
    stopAllTasks();
    threadPool->waitForDone();
    db->closeDatabase();
    delete db;
    delete taskQueue;
    delete queueMutex;
    delete queueCondition;
}

/*
 * Summary: 开始文件搜索
 * Parameters:
 * const QString &keyword - 搜索关键字
 * const QString &path - 搜索路径
 * Return: void
 */
void FileSearchCore::startSearch(const QString &keyword, const QString &path) {
    if(!uniqueFiles.isEmpty()){
        uniqueFiles.clear();
    }

    if(!uniquePaths.isEmpty()){
        uniquePaths.clear();
    }

    if (keyword.isEmpty()) {
        LOG_INFO("搜索关键字为空。");
        return;
    }

    QString searchPath = path;
    if (searchPath.isEmpty()) {
        searchPath = QDir::rootPath();
    } else {
        QDir dir(searchPath);
        if (!dir.exists()) {
            LOG_INFO("指定的路径不存在。");
            return;
        }
    }

    timer.start();
    LOG_INFO("搜索计时开始。");
    activeTaskCount = 0;
    updateCounter = 0;
    totalDirectories = 0;
    isSearching = true;

    // 使用数据库进行搜索
    QVector<QString> results = db->searchFiles(keyword);

    if (!results.isEmpty()) {
        for (const QString &filePath : results) {
            emit fileFound(filePath);
        }
        emit searchFinished();
    } else {
        LOG_INFO("数据库中没有结果，开始文件系统遍历搜索。");
        uniquePaths.clear();
        uniqueFiles.clear();
        enqueueDirectories(searchPath, 2);

        totalDirectories = taskQueue->size();
        emit progressUpdated(0, totalDirectories);

        for (int i = 0; i < threadPool->maxThreadCount(); ++i) {
            FileSearchThread *task = new FileSearchThread(keyword, taskQueue, queueMutex, queueCondition);
            connect(task, &FileSearchThread::fileFound, this, &FileSearchCore::onFileFound);
            connect(task, &FileSearchThread::searchFinished, this, &FileSearchCore::onSearchFinished);
            connect(task, &FileSearchThread::taskStarted, this, &FileSearchCore::onTaskStarted);
            threadPool->start(task);
        }
    }
}

/*
 * Summary: 将目录加入任务队列
 * Parameters:
 * const QString &path - 目录路径
 * int depth - 深度
 * Return: void
 */
void FileSearchCore::enqueueDirectories(const QString &path, int depth) {
    QDirIterator dirIt(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (dirIt.hasNext()) {
        dirIt.next();
        QString subDirPath = dirIt.filePath();
        if (!uniquePaths.contains(subDirPath)) {
            uniquePaths.insert(subDirPath);
            taskQueue->enqueue(subDirPath);
            totalDirectories++;

            if (depth > 1) {
                enqueueDirectories(subDirPath, depth - 1);
            }
        }
    }
}

/*
 * Summary: 处理找到的文件
 * Parameters:
 * const QString &filePath - 文件路径
 * Return: void
 */
void FileSearchCore::onFileFound(const QString &filePath) {
    if (!uniqueFiles.contains(filePath)) {
        uniqueFiles.insert(filePath);
        filesBatch.append(filePath);

        // 插入文件信息到数据库
        dbThread->insertFileInfo(filePath);

        emit fileFound(filePath);
    }
}

/*
 * Summary: 处理搜索完成的操作
 * Parameters: 无
 * Return: void
 */
void FileSearchCore::onSearchFinished() {
    QMutexLocker locker(queueMutex);
    activeTaskCount--;
    emit progressUpdated(totalDirectories - activeTaskCount, totalDirectories);

    if (activeTaskCount == 0 && taskQueue->isEmpty()) {
        finishSearch();
        qint64 elapsedTime = timer.elapsed();
        onSearchTime(elapsedTime);
        isSearching = false;
        emit searchFinished();
    }
}

/*
 * Summary: 完成搜索，进行清理工作
 * Parameters: 无
 * Return: void
 */
void FileSearchCore::finishSearch() {
    // 可以在这里进行一些清理工作
}

/*
 * Summary: 停止所有任务
 * Parameters: 无
 * Return: void
 */
void FileSearchCore::stopAllTasks() {
    QMutexLocker locker(queueMutex);
    isStopping = true;
    while (!taskQueue->isEmpty()) {
        taskQueue->dequeue();
    }
    queueCondition->wakeAll();
}

/*
 * Summary: 停止文件搜索
 * Parameters: 无
 * Return: void
 */
void FileSearchCore::stopSearch() {
    if (!isSearching) {
        LOG_INFO("没有正在执行的搜索任务。");
        return;
    }

    stopAllTasks();
    qint64 elapsedTime = timer.elapsed();
    LOG_INFO(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    timer.invalidate();

    finishSearch();

    isSearching = false;
    emit searchFinished();
}

/*
 * Summary: 处理搜索时间
 * Parameters:
 * qint64 elapsedTime - 已耗时间
 * Return: void
 */
void FileSearchCore::onSearchTime(qint64 elapsedTime) {
    LOG_INFO(QString("搜索耗时: %1 毫秒").arg(elapsedTime));
}

/*
 * Summary: 处理任务开始的操作
 * Parameters: 无
 * Return: void
 */
void FileSearchCore::onTaskStarted() {
    QMutexLocker locker(queueMutex);
    activeTaskCount++;
}



/*
 * Summary: 处理文件插入后的操作
 * Parameters:
 * const QString &filePath - 文件路径
 * Return: void
 */
void FileSearchCore::onFileInserted(const QString &filePath) {
    // 可以在这里处理文件插入后的操作
}

/*
 * Summary: 初始化文件数据库
 * Parameters: 无
 * Return: void
 */
void FileSearchCore::initFileDatabase() {
    QString rootPath = QDir::rootPath();

    // 清空已处理路径和文件的记录
    uniqueFiles.clear();
    uniquePaths.clear();

    // 遍历文件夹并建立索引
    enqueueDirectories(rootPath, 2); // 加入任务队列
    totalDirectories = taskQueue->size();

    // 只在后台运行数据库插入逻辑，避免更新UI
    for (int i = 0; i < threadPool->maxThreadCount(); ++i) {
        FileSearchThread *task = new FileSearchThread("", taskQueue, queueMutex, queueCondition);
        connect(task, &FileSearchThread::fileFound, this, [this](const QString &filePath) {
            if (!uniqueFiles.contains(filePath)) {
                uniqueFiles.insert(filePath);
                // 插入文件信息到数据库而不更新UI
                dbThread->insertFileInfo(filePath);
            }
        });
        threadPool->start(task);
    }
    LOG_INFO("文件索引数据库建立启动完成。");
}
