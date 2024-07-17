#include "FileSearchThread.h"

FileSearchThread::FileSearchThread(const QString &keyword, const QString &path, QObject *parent)
        : QThread(parent), searchKeyword(keyword), searchPath(path) {}

void FileSearchThread::run() {
    QDirIterator it(searchPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (filePath.contains(searchKeyword, Qt::CaseInsensitive)) {
            emit fileFound(filePath);
        }
    }
    emit searchFinished();
}
