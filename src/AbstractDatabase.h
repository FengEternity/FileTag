/*
 * AbstractDatabase.h
 * Author: Montee
 * CreateDate: 2024-11-1
 * Updater: Montee
 * UpdateDate: 2024-11-1
 * Summary: 数据库纯虚基类，用于继承扩展
 */

#ifndef ABSTRACTDATABASE_H
#define ABSTRACTDATABASE_H

#include <QString>
#include <QVector>

class AbstractDatabase {
public:
    explicit AbstractDatabase(const QString &dbName);
    virtual ~AbstractDatabase();

    virtual bool openDatabase() = 0;    // 打开数据库
    virtual void closeDatabase() = 0;   // 关闭数据库
    virtual bool createTables() = 0;    // 创建表，返回是否成功

protected:
    QString databaseName; // 数据库文件名
};

#endif // ABSTRACTDATABASE_H

