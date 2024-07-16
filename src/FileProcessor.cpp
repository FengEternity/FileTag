#include "FileProcessor.h"
#include <QFileInfo>
#include <QFileInfoList>
#include <QTextStream>

FileProcessor::FileProcessor(QObject *parent)
        : QObject(parent), currentFile(nullptr), networkManager(new QNetworkAccessManager(this)), workerThread(new QThread)
{
    moveToThread(workerThread);
    connect(workerThread, &QThread::finished, this, &QObject::deleteLater);
    workerThread->start();
}

FileProcessor::~FileProcessor()
{
    workerThread->quit();
    workerThread->wait();
}

void FileProcessor::searchFiles(const QString &directory, const QString &keyword)
{
    searchDirectory = directory;
    searchKeyword = keyword;
    QMetaObject::invokeMethod(this, "onSearchFiles", Qt::QueuedConnection);
}

void FileProcessor::uploadFile(const QString &filePath, const QUrl &url)
{
    currentFile = new QFile(filePath, this);
    if (!currentFile->open(QIODevice::ReadOnly)) {
        emit uploadFinished();
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->post(request, currentFile);
    connect(reply, &QNetworkReply::uploadProgress, this, &FileProcessor::uploadProgress);
    connect(reply, &QNetworkReply::finished, this, &FileProcessor::onUploadFinished);
}

void FileProcessor::downloadFile(const QUrl &url, const QString &savePath)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, &FileProcessor::downloadProgress);
    connect(reply, &QNetworkReply::finished, this, &FileProcessor::onDownloadFinished);

    currentFile = new QFile(savePath, this);
    if (!currentFile->open(QIODevice::WriteOnly)) {
        emit downloadFinished();
        return;
    }
}

void FileProcessor::onSearchFiles()
{
    searchResults.clear();
    searchInDirectory(QDir(searchDirectory), searchKeyword, searchResults);
    emit searchFinished(searchResults);
}

void FileProcessor::onUploadFinished()
{
    currentFile->close();
    currentFile->deleteLater();
    emit uploadFinished();
}

void FileProcessor::onDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply) {
        currentFile->write(reply->readAll());
        currentFile->close();
        currentFile->deleteLater();
        emit downloadFinished();
    }
}

void FileProcessor::searchInDirectory(const QDir &dir, const QString &keyword, QStringList &results)
{
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::AllDirs);
            foreach (const QFileInfo &info, list) {
            if (info.isDir()) {
                searchInDirectory(info.filePath(), keyword, results);
            } else {
                QFile file(info.filePath());
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QString content = in.readAll();
                    if (content.contains(keyword, Qt::CaseInsensitive)) {
                        results.append(info.filePath());
                    }
                }
            }
        }
}
