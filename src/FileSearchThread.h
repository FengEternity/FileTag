// FileSearchThread.h

#ifndef FILESEARCHTHREAD_H
#define FILESEARCHTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class FileSearchThread : public QObject, public QRunnable {
Q_OBJECT
public:
    explicit FileSearchThread(const QString &keyword, QQueue<QString> *taskQueue, QMutex *queueMutex, QWaitCondition *queueCondition, QObject *parent = nullptr);
    ~FileSearchThread();
    void run() override; // 继承 QRunnable 的 run 方法
    void stop();

signals:
    void fileFound(const QString &filePath);
    void searchFinished();

private:
    QString searchKeyword;
    QQueue<QString> *taskQueue;
    QMutex *queueMutex;
    QWaitCondition *queueCondition;
    bool stopped;
};

#endif // FILESEARCHTHREAD_H
