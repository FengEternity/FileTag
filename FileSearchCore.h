/*
 * FileSearchCore.h
 * Author: Montee
 * CreateDate: 2024-11-1
 * Updater: Montee
 * UpdateDate: 2024-11-1
 * Summary: 文件文件搜索核心逻辑
 */

#ifndef FILESEARCHCORE_H
#define FILESEARCHCORE_H

#include <QObject>
#include <QThreadPool>
#include <QElapsedTimer>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>

#include "FileSearchThread.h"
#include "FileIndexDatabase.h"
#include "DatabaseThread.h"

class FileSearchCore : public QObject {
Q_OBJECT

public:
    explicit FileSearchCore(QObject *parent = nullptr);
    ~FileSearchCore();

    void startSearch(const QString &keyword, const QString &path, bool includeSystemFiles);
    void stopSearch();
    void initFileDatabase();
    bool isSystemDirectory(const QString& path);
signals:
    void fileFound(const QString &filePath);
    void searchFinished();
    void progressUpdated(int value, int total);

private slots:
    void onFileInserted(const QString &filePath);
    void onSearchFinished();
    void onTaskStarted();
    void onFileFound(const QString &filePath);

private:
    void enqueueDirectories(const QString &path, int depth, bool includeSystemFiles);
    void finishSearch();
    void stopAllTasks();
    void onSearchTime(qint64 elapsedTime);

    // 成员变量
    int activeTaskCount;
    int totalDirectories;
    int updateCounter;
    bool isSearching;
    bool firstSearch;
    bool isStopping;
    bool includeSystemFiles;
    static QVector<QString> filesBatch;

    QThreadPool *threadPool;
    QElapsedTimer timer;
    QSet<QString> uniquePaths;
    QSet<QString> uniqueFiles;
    QQueue<QString> *taskQueue;
    QMutex *queueMutex;
    QWaitCondition *queueCondition;
    QMutex uniqueFilesMutex;

    FileIndexDatabase *db;
    DatabaseThread *dbThread;
};

#endif // FILESEARCHCORE_H
