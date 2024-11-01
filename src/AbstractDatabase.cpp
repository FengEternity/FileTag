/*
 * AbstractDatabase.h
 * Author: Montee
 * CreateDate: 2024-11-1
 * Updater: Montee
 * UpdateDate: 2024-11-1
 * Summary: 数据库纯虚基类，用于继承扩展
 */

#include "AbstractDatabase.h"

AbstractDatabase::AbstractDatabase(const QString &dbName) : databaseName(dbName) {}

AbstractDatabase::~AbstractDatabase() {}
