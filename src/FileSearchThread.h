#ifndef FILESEARCHTHREAD_H
#define FILESEARCHTHREAD_H

#include <QThread>
#include <QString>
#include <QDirIterator>
#include <QMutex>
#include <QElapsedTimer> // 新增

class FileSearchThread : public QThread {
Q_OBJECT

public:
    FileSearchThread(const QString &keyword, const QString &path, QObject *parent = nullptr);
    void run() override;
    void stop();  // 停止线程的方法

signals:
    void fileFound(const QString &filePath);
    void searchFinished();
    void searchTime(qint64 elapsedTime); // 新增的信号

private:
    QString searchKeyword;
    QString searchPath;
    bool stopped;
    QMutex mutex;
    QElapsedTimer timer; // 新增
};

#endif // FILESEARCHTHREAD_H
