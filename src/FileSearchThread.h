#ifndef FILESEARCHTHREAD_H
#define FILESEARCHTHREAD_H

#include <QThread>
#include <QString>
#include <QDirIterator>
#include <QMutex>

class FileSearchThread : public QThread {
Q_OBJECT

public:
    FileSearchThread(const QString &keyword, const QString &path, QObject *parent = nullptr);
    void run() override;
    void stop();  // 停止线程的方法

signals:
    void fileFound(const QString &filePath);
    void searchFinished();

private:
    QString searchKeyword;
    QString searchPath;
    bool stopped;
    QMutex mutex;
};

#endif // FILESEARCHTHREAD_H
