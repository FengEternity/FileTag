// FileSearch.cpp

#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include "FileSearchThread.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// 定义静态成员变量
QVector<QString> FileSearch::filesBatch;

FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        threadPool(new QThreadPool(this)),
        updateCounter(0),
        activeTaskCount(0),
        totalDirectories(0),
        isSearching(false),
        firstSearch(true),
        db(new FileDatabase("file_index.db"))
{
    // 设置 UI
    ui->setupUi(this);

    // 连接界面元素到成员变量
    searchButton = ui->searchButton;
    searchLineEdit = ui->searchLineEdit;
    pathLineEdit = ui->pathLineEdit;
    filterLineEdit = ui->filterLineEdit;
    resultTableView = ui->resultTableView;
    // 设置表格视图模型
    tableModel = new QStandardItemModel(this);
    tableModel->setHorizontalHeaderLabels({"序号", "文件名", "文件路径", "文件类型", "创建时间", "修改时间"});

    // 设置代理模型
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(tableModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1);
    proxyModel->setSortRole(Qt::UserRole);

    // 将代理模型设置到表格视图
    resultTableView->setModel(proxyModel);
    resultTableView->horizontalHeader()->setStretchLastSection(true);
    resultTableView->setSortingEnabled(true);
    resultTableView->sortByColumn(0, Qt::AscendingOrder);


    // 连接界面元素到槽函数
    finishButton = ui->finishButton;
    progressBar = ui->progressBar;
    progressLabel = ui->progressLabel;

    connect(searchButton, &QPushButton::clicked, this, &FileSearch::onSearchButtonClicked);
    connect(finishButton, &QPushButton::clicked, this, &FileSearch::onFinishButtonClicked);
    connect(filterLineEdit, &QLineEdit::textChanged, this, &FileSearch::onSearchFilterChanged);

    LOG_INFO("表格视图模型设置完成。");

    if (!layout()) {
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(filterLineEdit);
        layout->addWidget(resultTableView);
        setLayout(layout);
    }

    threadPool->setMaxThreadCount(QThread::idealThreadCount());
    LOG_INFO("线程池初始化完成, 最大线程数: " + QString::number(threadPool->maxThreadCount()));

    // 初始化任务队列和相关同步机制
    taskQueue = new QQueue<QString>();
    queueMutex = new QMutex();
    queueCondition = new QWaitCondition();
    LOG_INFO("任务队列和同步机制初始化完成。");

    // 初始化数据库并创建表
    if (db->openDatabase()) {
        LOG_INFO("数据库打开成功。");
        if (db->createTables()) {
            LOG_INFO("数据库表创建成功。");
        } else {
            LOG_ERROR("数据库表创建失败。");
        }
    } else {
        LOG_ERROR("数据库打开失败。");
    }
}

FileSearch::~FileSearch() {
    stopAllTasks();
    threadPool->waitForDone();
    db->closeDatabase();
    delete db;
    delete ui;
    delete taskQueue;
    delete queueMutex;
    delete queueCondition;
}



void FileSearch::onSearchButtonClicked() {
    QString searchKeyword = searchLineEdit->text();
    QString searchPath = pathLineEdit->text();

    if (searchKeyword.isEmpty()) {
        QMessageBox::information(this, "搜索关键字为空", "请输入搜索关键字。");
        return;
    }

    if (searchPath.isEmpty()) {
        searchPath = QDir::rootPath();
    } else {
        QDir dir(searchPath);
        if (!dir.exists()) {
            QMessageBox::information(this, "路径错误", "指定的路径不存在，请重新输入。");
            return;
        }
    }

    tableModel->removeRows(0, tableModel->rowCount());
    timer.start();
    LOG_INFO("搜索计时开始。");
    activeTaskCount = 0;
    updateCounter = 0;
    totalDirectories = 0;
    isSearching = true;

    // 使用数据库进行搜索
    QVector<QString> results = db->searchFiles(searchKeyword);

    if (!results.isEmpty()) {
        for (const QString &filePath : results) {
            onFileFound(filePath);  // 利用已有逻辑来显示搜索结果
        }
    } else {
        LOG_INFO("没有找到符合条件的文件，进行文件系统遍历搜索。");
        // 如果数据库中没有结果，则进行文件系统遍历搜索
        uniquePaths.clear();
        uniqueFiles.clear();
        enqueueDirectories(searchPath, 2);

        progressBar->setMaximum(totalDirectories);
        progressBar->setValue(0);
        updateProgressLabel();

        for (int i = 0; i < threadPool->maxThreadCount(); ++i) {
            FileSearchThread *task = new FileSearchThread(searchKeyword, taskQueue, queueMutex, queueCondition);
            connect(task, &FileSearchThread::fileFound, this, &FileSearch::onFileFound);
            connect(task, &FileSearchThread::searchFinished, this, &FileSearch::onSearchFinished);
            connect(task, &FileSearchThread::taskStarted, this, &FileSearch::onTaskStarted);
            threadPool->start(task);
        }
    }
}


void FileSearch::enqueueDirectories(const QString &path, int depth) {
    QDirIterator dirIt(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (dirIt.hasNext()) {
        dirIt.next();
        QString subDirPath = dirIt.filePath();
        if (!uniquePaths.contains(subDirPath)) {
            uniquePaths.insert(subDirPath);
            taskQueue->enqueue(subDirPath);
            totalDirectories++;

            if (depth > 1) {
                QDirIterator subDirIt(subDirPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
                while (subDirIt.hasNext()) {
                    subDirIt.next();
                    QString subSubDirPath = subDirIt.filePath();
                    if (!uniquePaths.contains(subSubDirPath)) {
                        uniquePaths.insert(subSubDirPath);
                        taskQueue->enqueue(subSubDirPath);
                        totalDirectories++;
                    }
                }
            }
        }
    }
}

void FileSearch::onFileFound(const QString &filePath) {
    if (!uniqueFiles.contains(filePath)) {
        uniqueFiles.insert(filePath);
        filesBatch.append(filePath);

        // 插入文件信息到数据库
        if(db->insertFileInfo(filePath)) {
            int fileId = db->getFileId(filePath);
            if(fileId > 0) {
                // 插入文件关键词到数据库
                QVector<QString> keywords = extractKeywordsFromFile(filePath); // 提取关键词
                db->insertFileKeywords(fileId, keywords);
            } else {
                LOG_ERROR("获取文件 ID 失败: " + filePath);
            }
        }

        // 如果文件数目不足500，则直接更新
        if (firstSearch || filesBatch.size() < 500) {
            QVector<QString> filesBatchCopy = filesBatch;
            filesBatch.clear();

            auto updateUI = [this, filesBatchCopy]() {
                int currentRowCount = tableModel->rowCount();
                for (const QString &filePath : filesBatchCopy) {
                    QFileInfo fileInfo(filePath);
                    QList<QStandardItem *> items;

                    QStandardItem *item0 = new QStandardItem(QString::number(++currentRowCount));
                    item0->setData(currentRowCount, Qt::UserRole);
                    items.append(item0);

                    QStandardItem *item1 = new QStandardItem(fileInfo.fileName());
                    item1->setData(fileInfo.fileName(), Qt::UserRole);
                    items.append(item1);

                    QStandardItem *item2 = new QStandardItem(fileInfo.absoluteFilePath());
                    item2->setData(fileInfo.absoluteFilePath(), Qt::UserRole);
                    items.append(item2);

                    QStandardItem *item3 = new QStandardItem(fileInfo.suffix());
                    item3->setData(fileInfo.suffix(), Qt::UserRole);
                    items.append(item3);

                    QStandardItem *item4 = new QStandardItem(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
                    item4->setData(fileInfo.birthTime(), Qt::UserRole);
                    items.append(item4);

                    QStandardItem *item5 = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
                    item5->setData(fileInfo.lastModified(), Qt::UserRole);
                    items.append(item5);

                    tableModel->appendRow(items);
                }
            };
            QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
            firstSearch = false;
        } else {
            // 超过500后，每1000次更新一次
            if (++updateCounter % 1000 == 0) {
                QVector<QString> filesBatchCopy = filesBatch;
                filesBatch.clear();

                auto updateUI = [this, filesBatchCopy]() {
                    int currentRowCount = tableModel->rowCount();
                    for (const QString &filePath : filesBatchCopy) {
                        QFileInfo fileInfo(filePath);
                        QList<QStandardItem *> items;

                        QStandardItem *item0 = new QStandardItem(QString::number(++currentRowCount));
                        item0->setData(currentRowCount, Qt::UserRole);
                        items.append(item0);

                        QStandardItem *item1 = new QStandardItem(fileInfo.fileName());
                        item1->setData(fileInfo.fileName(), Qt::UserRole);
                        items.append(item1);

                        QStandardItem *item2 = new QStandardItem(fileInfo.absoluteFilePath());
                        item2->setData(fileInfo.absoluteFilePath(), Qt::UserRole);
                        items.append(item2);

                        QStandardItem *item3 = new QStandardItem(fileInfo.suffix());
                        item3->setData(fileInfo.suffix(), Qt::UserRole);
                        items.append(item3);

                        QStandardItem *item4 = new QStandardItem(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
                        item4->setData(fileInfo.birthTime(), Qt::UserRole);
                        items.append(item4);

                        QStandardItem *item5 = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
                        item5->setData(fileInfo.lastModified(), Qt::UserRole);
                        items.append(item5);

                        tableModel->appendRow(items);
                    }
                };
                QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
            }
        }
    }
}

void FileSearch::finishSearch() {
    if (!filesBatch.isEmpty()) {
        QVector<QString> filesBatchCopy = filesBatch;
        filesBatch.clear();

        auto updateUI = [this, filesBatchCopy]() {
            int currentRowCount = tableModel->rowCount();
            for (const QString &filePath : filesBatchCopy) {
                QFileInfo fileInfo(filePath);
                QList<QStandardItem *> items;

                QStandardItem *item0 = new QStandardItem(QString::number(++currentRowCount));
                item0->setData(currentRowCount, Qt::UserRole);
                items.append(item0);

                QStandardItem *item1 = new QStandardItem(fileInfo.fileName());
                item1->setData(fileInfo.fileName(), Qt::UserRole);
                items.append(item1);

                QStandardItem *item2 = new QStandardItem(fileInfo.absoluteFilePath());
                item2->setData(fileInfo.absoluteFilePath(), Qt::UserRole);
                items.append(item2);

                QStandardItem *item3 = new QStandardItem(fileInfo.suffix());
                item3->setData(fileInfo.suffix(), Qt::UserRole);
                items.append(item3);

                QStandardItem *item4 = new QStandardItem(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
                item4->setData(fileInfo.birthTime(), Qt::UserRole);
                items.append(item4);

                QStandardItem *item5 = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
                item5->setData(fileInfo.lastModified(), Qt::UserRole);
                items.append(item5);

                tableModel->appendRow(items);
            }
        };
        QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
    }
}

void FileSearch::onSearchFinished() {
    QMutexLocker locker(queueMutex);
    activeTaskCount--;
    // Logger::instance().log("activeTaskCount 减少，当前计数: " + QString::number(activeTaskCount));
    progressBar->setValue(progressBar->value() + 1);
    updateProgressLabel();

    if (activeTaskCount == 0 && taskQueue->isEmpty()) {
        finishSearch();
        qint64 elapsedTime = timer.elapsed();
        onSearchTime(elapsedTime);
        progressBar->setValue(totalDirectories);
        updateProgressLabel();
        isSearching = false;
    }
}

void FileSearch::updateProgressLabel() {
    int percentage = static_cast<int>((static_cast<float>(progressBar->value()) / totalDirectories) * 100);
    progressLabel->setText(QString::number(percentage) + "%");
}

void FileSearch::onSearchTime(qint64 elapsedTime) {
    if(!isStopping){
        QMessageBox::information(this, "搜索完成", QString("搜索耗时: %1 毫秒").arg(elapsedTime));
    }
    timer.invalidate();
    LOG_INFO(QString("计时结束，搜索耗时: %1 毫秒").arg(elapsedTime));
}

void FileSearch::onFinishButtonClicked() {
    if (!isSearching) {
        LOG_ERROR("没有正在执行的搜索任务。");
        QMessageBox::information(this, "搜索中断", "没有正在执行的搜索任务。");
        return;
    }

    stopAllTasks();
    qint64 elapsedTime = timer.elapsed();
    if (!isStopping){
        QMessageBox::information(this, "搜索中断", QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    }
    timer.invalidate();
    LOG_INFO(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));

    finishSearch();

    progressBar->setValue(progressBar->maximum());
    updateProgressLabel();
    isSearching = false;
}

void FileSearch::stopAllTasks() {
    QMutexLocker locker(queueMutex);
    isStopping = true;
    while (!taskQueue->isEmpty()) {
        taskQueue->dequeue();
    }
    queueCondition->wakeAll();
}

void FileSearch::onSearchFilterChanged(const QString &text) {
    proxyModel->setFilterWildcard(text);
}

void FileSearch::onTaskStarted() {
    QMutexLocker locker(queueMutex);
    activeTaskCount++;
    // Logger::instance().log("activeTaskCount 增加，当前计数: " + QString::number(activeTaskCount));
}

QVector<QString> FileSearch::extractKeywordsFromFile(const QString &filePath) {
    QVector<QString> keywords;

    QString filename = QFileInfo(filePath).fileName();

    // 使用正则表达式匹配所有单词
    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(filename.toLower());

    // 停用词列表（可以根据需要添加更多停用词）
    QSet<QString> stopwords = {"the", "and", "is", "in", "to", "of", "a", "an"};

    // 提取并过滤关键词
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);

        // 过滤掉长度小于等于2的词和停用词
        if (word.length() > 2 && !stopwords.contains(word)) {
            keywords.append(word);
        }
    }

    return keywords;
}