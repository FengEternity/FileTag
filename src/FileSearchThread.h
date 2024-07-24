#ifndef FILESEARCHTHREAD_H
#define FILESEARCHTHREAD_H

#include <QRunnable>
#include <QObject>
#include <QString>
#include <QDirIterator>
#include <QElapsedTimer> // 新增

class FileSearchThread : public QObject, public QRunnable {
Q_OBJECT

public:
    FileSearchThread(const QString &keyword, const QString &path, QObject *parent = nullptr);
    void run() override;
    void stop();
    ~FileSearchThread();

signals:
    void fileFound(const QString &filePath);
    void searchFinished();
    void searchTime(qint64 elapsedTime); // 新增的信号

private:
    QString searchKeyword;
    QString searchPath;
    QElapsedTimer timer;
    bool stopped;
};

#endif // FILESEARCHTHREAD_H
