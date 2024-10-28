#include "FileDatabase.h"
#include "Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>

// 构造函数：初始化数据库文件名
FileDatabase::FileDatabase(const QString &dbName) : databaseName(dbName) {}

// 析构函数：关闭数据库连接
FileDatabase::~FileDatabase() {
    closeDatabase();
}

// 打开数据库连接并创建表
bool FileDatabase::openDatabase() {
    // 检查是否已经有同名的连接
    if (QSqlDatabase::contains("file_db_connection")) {
        db = QSqlDatabase::database("file_db_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "file_db_connection");
        db.setDatabaseName(databaseName);
    }

    if (!db.open()) {
        QString errorMessage = QString("无法打开数据库: %1").arg(db.lastError().text());
        qDebug() << errorMessage;
        LOG_ERROR(errorMessage);
        return false; // 如果连接失败，返回 false
    }

    LOG_INFO("数据库连接成功：" + databaseName);
    return createTables(); // 连接成功后，创建表
}

// 关闭数据库连接
void FileDatabase::closeDatabase() {
    if (db.isOpen()) {
        db.close(); // 关闭数据库连接
        LOG_INFO("数据库连接已关闭。");
        qDebug() << "数据库连接已关闭。";
    }
    QSqlDatabase::removeDatabase("file_db_connection");
}

// 创建表，返回是否成功
bool FileDatabase::createTables() {
    if (!db.isOpen()) {
        LOG_ERROR("数据库未打开，无法创建表。");
        return false;
    }

    QSqlQuery query(db); // 使用与数据库连接绑定的查询对象

    QString sqlCreateFiles = R"(
        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT UNIQUE,
            name TEXT,
            extension TEXT,
            birth_time TEXT,
            last_modified TEXT
        )
    )";
    if (!query.exec(sqlCreateFiles)) {
        QString errorMessage = QString("创建 files 表失败: %1").arg(query.lastError().text());
        qDebug() << errorMessage;
        LOG_ERROR(errorMessage);
        return false;
    }

    QString sqlCreateFileKeywords = R"(
        CREATE TABLE IF NOT EXISTS file_keywords (
            file_id INTEGER,
            keyword TEXT,
            FOREIGN KEY (file_id) REFERENCES files(id)
        )
    )";
    if (!query.exec(sqlCreateFileKeywords)) {
        QString errorMessage = QString("创建 file_keywords 表失败: %1").arg(query.lastError().text());
        qDebug() << errorMessage;
        LOG_ERROR(errorMessage);
        return false;
    }

    LOG_INFO("数据库表创建成功。");
    qDebug() << "数据库表创建成功。";
    return true;
}

// 插入文件信息，返回是否成功
bool FileDatabase::insertFileInfo(const QString &filePath) {
    if (!db.isOpen()) {
        LOG_ERROR("数据库未打开，无法插入文件信息。");
        return false;
    }

    QFileInfo fileInfo(filePath);
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT OR REPLACE INTO files (path, name, extension, birth_time, last_modified)
        VALUES (?, ?, ?, ?, ?)
    )");
    query.addBindValue(fileInfo.absoluteFilePath());
    query.addBindValue(fileInfo.fileName());
    query.addBindValue(fileInfo.suffix());
    query.addBindValue(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.addBindValue(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));

    if (!query.exec()) {
        QString errorMessage = QString("插入文件信息失败: %1").arg(query.lastError().text());
        qDebug() << errorMessage;
        LOG_ERROR(errorMessage);
        return false;
    }

    LOG_INFO("文件信息插入成功：" + filePath);
    qDebug() << "文件信息插入成功: " << filePath;
    return true;
}

// 插入关键词到数据库中
void FileDatabase::insertFileKeywords(int fileId, const QVector<QString> &keywords) {
    if (!db.isOpen()) {
        LOG_ERROR("数据库未打开，无法插入关键词。");
        return;
    }

    QSqlQuery query(db);
    for (const QString &keyword : keywords) {
        query.prepare("INSERT INTO file_keywords (file_id, keyword) VALUES (?, ?)");
        query.addBindValue(fileId);
        query.addBindValue(keyword);

        if (!query.exec()) {
            QString errorMessage = QString("插入关键词失败，文件 ID: %1, 关键词: %2, 错误信息: %3")
                    .arg(fileId)
                    .arg(keyword)
                    .arg(query.lastError().text());
            qDebug() << errorMessage;
            LOG_ERROR(errorMessage);
        } else {
            LOG_INFO("关键词插入成功，文件 ID: " + QString::number(fileId) + ", 关键词: " + keyword);
            qDebug() << "关键词插入成功，文件 ID: " << fileId << ", 关键词: " << keyword;
        }
    }
}

// 根据关键词从数据库中搜索文件路径
QVector<QString> FileDatabase::searchFiles(const QString &keyword) {
    QVector<QString> resultPaths;
    if (!db.isOpen()) {
        LOG_ERROR("数据库未打开，无法搜索文件。");
        return resultPaths;
    }

    QSqlQuery query(db);
    QString sql = R"(
        SELECT path FROM files
        WHERE path LIKE '%' || ? || '%'
        OR name LIKE '%' || ? || '%'
        OR EXISTS (
            SELECT 1 FROM file_keywords
            WHERE file_keywords.file_id = files.id
            AND file_keywords.keyword LIKE ?
        )
    )";
    query.prepare(sql);
    query.addBindValue(keyword);
    query.addBindValue(keyword);
    query.addBindValue(keyword);

    if (query.exec()) {
        while (query.next()) {
            resultPaths.append(query.value(0).toString());
        }
        LOG_INFO(QString("搜索完成，找到 %1 个匹配文件。").arg(resultPaths.size()));
        qDebug() << QString("搜索完成，找到 %1 个匹配文件。").arg(resultPaths.size());
    } else {
        QString errorMessage = QString("执行搜索查询失败: %1").arg(query.lastError().text());
        qDebug() << errorMessage;
        LOG_ERROR(errorMessage);
    }

    return resultPaths;
}

// 获取文件路径对应的数据库ID
int FileDatabase::getFileId(const QString &filePath) {
    if (!db.isOpen()) {
        LOG_ERROR("数据库未打开，无法获取文件 ID。");
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id FROM files WHERE path = ?");
    query.addBindValue(filePath);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    QString errorMessage = "未找到文件ID：" + filePath;
    qDebug() << errorMessage;
    LOG_INFO(errorMessage);
    return -1;
}
