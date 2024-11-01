//
// Created by Monty-Lee  on 24-10-31.
//

#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QVariant>

#include "AbstractDatabase.h"

class DatabaseThread : public QThread {
Q_OBJECT
public:
    explicit DatabaseThread(AbstractDatabase *db, QObject *parent = nullptr);
    ~DatabaseThread();

    void addInsertFileTask(const QString &filePath);
    void addSearchFilesTask(const QString &keyword);

signals:
    void fileInserted(const QString &filePath);
    void searchFinished(const QVector<QString> &results);

protected:
    void run() override;

private:
    struct Task {
        enum TaskType { InsertFile, SearchFiles } type;
        QVariant data;
    };

    AbstractDatabase *db;
    QQueue<Task> taskQueue;
    QMutex mutex;
    QWaitCondition condition;
    bool isRunning;

    void processInsertFile(const QString &filePath);
    void processSearchFiles(const QString &keyword);
};

#endif // DATABASETHREAD_H
