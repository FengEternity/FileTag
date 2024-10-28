//
// Created by Monty-Lee  on 24-10-27.
//

#ifndef FILEDATABASE_H
#define FILEDATABASE_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QVector>
#include <QDebug>

class FileDatabase {
public:
    explicit FileDatabase(const QString &dbName);
    ~FileDatabase();

    bool openDatabase(); // 打开数据库
    void closeDatabase(); // 关闭数据库
    bool createTables(); // 创建表，返回是否成功
    bool insertFileInfo(const QString &filePath); // 插入文件信息，返回是否成功
    void insertFileKeywords(int fileId, const QVector<QString> &keywords); // 插入关键词
    QVector<QString> searchFiles(const QString &keyword); // 搜索文件
    int getFileId(const QString &filePath); // 获取文件ID

private:
    QString databaseName; // 数据库文件名
    QSqlDatabase db;      // 数据库连接对象
};

#endif // FILEDATABASE_H
