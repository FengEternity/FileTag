//
// Created by Monty-Lee  on 24-10-31.
//

#ifndef FILETAG_DATABASETHREAD_H
#define FILETAG_DATABASETHREAD_H

#include<QThread>
#include "FileDatabase.h"

class DatabaseThread : public QThread {
Q_OBJECT

public:
    explicit DatabaseThread(FileDatabase *db, QObject *parent = nullptr);
    ~DatabaseThread() override;

    void run() override;

    void insertFileInfo(const QString &filePath);
    void searchFiles(const QString &keyword);

signals:
    void fileInserted(const QString &filePath);
    void searchFinished(const QVector<QString> &result);

private:
    FileDatabase *db;
    QString filePath;
    QString keyword;
};


#endif //FILETAG_DATABASETHREAD_H
