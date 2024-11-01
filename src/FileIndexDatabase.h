//
// Created by Monty-Lee  on 24-10-27.
//

#ifndef FILEDATABASE_H
#define FILEDATABASE_H

#include <QString>
#include <QSqlDatabase>
#include <QVector>

#include "AbstractDatabase.h"

class FileIndexDatabase : public AbstractDatabase {
public:
    explicit FileIndexDatabase(const QString &dbName);
    ~FileIndexDatabase() override;

    bool openDatabase() override;      // 打开数据库
    void closeDatabase() override;     // 关闭数据库
    bool createTables() override;      // 创建表，返回是否成功

    bool insertFileInfo(const QString &filePath);                 // 插入文件信息，返回是否成功
    void insertFileKeywords(int fileId, const QVector<QString> &keywords); // 插入关键词
    QVector<QString> searchFiles(const QString &keyword);         // 搜索文件
    int getFileId(const QString &filePath);                       // 获取文件ID

private:
    QSqlDatabase db; // 数据库连接对象
};

#endif // FILEDATABASE_H

