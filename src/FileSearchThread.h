#ifndef FILESEARCHTHREAD_H
#define FILESEARCHTHREAD_H

#include <QThread>
#include <QString>
#include <QDirIterator>

class FileSearchThread : public QThread {
Q_OBJECT

public:
    FileSearchThread(const QString &keyword, const QString &path, QObject *parent = nullptr);

    void run() override;

signals:
    void fileFound(const QString &filePath);
    void searchFinished();

private:
    QString searchKeyword;
    QString searchPath;
};

#endif // FILESEARCHTHREAD_H
