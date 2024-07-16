#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class FileProcessor : public QObject
{
Q_OBJECT
public:
    explicit FileProcessor(QObject *parent = nullptr);
    ~FileProcessor();

    void searchFiles(const QString &directory, const QString &keyword);
    void uploadFile(const QString &filePath, const QUrl &url);
    void downloadFile(const QUrl &url, const QString &savePath);

signals:
    void searchFinished(const QStringList &results);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadFinished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();

private slots:
    void onSearchFiles();
    void onUploadFinished();
    void onDownloadFinished();

private:
    void searchInDirectory(const QDir &dir, const QString &keyword, QStringList &results);

    QString searchDirectory;
    QString searchKeyword;
    QStringList searchResults;

    QFile *currentFile;
    QNetworkAccessManager *networkManager;
    QThread *workerThread;
};

#endif // FILEPROCESSOR_H
