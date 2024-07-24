#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include "FileSearchThread.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QMessageBox>

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
        firstSearch(true) // 初始化为 true
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
    proxyModel->setFilterKeyColumn(-1); // 过滤所有列

    // 将代理模型设置到表格视图
    proxyModel->setSortRole(Qt::UserRole); // 设置排序角色为用户角色
    resultTableView->setModel(proxyModel);
    resultTableView->horizontalHeader()->setStretchLastSection(true); // 自动调整最后一列的宽度
    resultTableView->setSortingEnabled(true);
    resultTableView->sortByColumn(0, Qt::AscendingOrder); // 按照序列号列排序

    // resultTableView->setColumnHidden(0, true); // 隐藏第一列

    // 连接界面元素到槽函数
    finishButton = ui->finishButton;
    progressBar = ui->progressBar;
    progressLabel = ui->progressLabel;

    connect(searchButton, &QPushButton::clicked, this, &FileSearch::onSearchButtonClicked);
    connect(finishButton, &QPushButton::clicked, this, &FileSearch::onFinishButtonClicked);
    connect(filterLineEdit, &QLineEdit::textChanged, this, &FileSearch::onSearchFilterChanged);

    Logger::instance().log("表格视图模型设置完成。");

    if (!layout()) {
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(filterLineEdit);
        layout->addWidget(resultTableView);
        setLayout(layout);
    }

    threadPool->setMaxThreadCount(QThread::idealThreadCount());
    Logger::instance().log("线程池初始化完成, 最大线程数: " + QString::number(threadPool->maxThreadCount()));
}

FileSearch::~FileSearch() {
    stopAllTasks(); // 确保析构时停止所有任务
    threadPool->waitForDone();
    delete ui;
}

void FileSearch::onSearchButtonClicked() {
    QString searchKeyword = searchLineEdit->text();
    QString searchPath = pathLineEdit->text();

    if (searchPath.isEmpty()) {
        searchPath = QDir::rootPath();
    }

    tableModel->removeRows(0, tableModel->rowCount());
    Logger::instance().log("Search started for keyword: " + searchKeyword + " in path: " + searchPath);

    timer.start();
    Logger::instance().log("搜索计时开始。");
    activeTaskCount = 0;
    updateCounter = 0;
    totalDirectories = 0;
    isSearching = true;

    // resultTableView->setSortingEnabled(false);  // 禁用排序功能

    QDirIterator dirIt(searchPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext()) {
        dirIt.next();
        totalDirectories++;
    }

    progressBar->setMaximum(totalDirectories);
    progressBar->setValue(0);
    updateProgressLabel();

    QDirIterator it(searchPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QString dirPath = it.next();
        auto *task = new FileSearchThread(searchKeyword, dirPath);
        connect(task, &FileSearchThread::fileFound, this, &FileSearch::onFileFound);
        connect(task, &FileSearchThread::searchFinished, this, &FileSearch::onSearchFinished);
        activeTasks.append(task); // 保存正在运行的任务
        activeTaskCount++;
        threadPool->start(task);
    }
}

void FileSearch::onFileFound(const QString &filePath) {
    filesBatch.append(filePath);
    Logger::instance().log("文件找到: " + filePath);

    if(firstSearch) {
        QVector<QString> filesBatchCopy = filesBatch;
        filesBatch.clear();

        auto updateUI = [this, filesBatchCopy]() {
            int currentRowCount = tableModel->rowCount();
            for (const QString &filePath : filesBatchCopy) {
                QFileInfo fileInfo(filePath);
                QList<QStandardItem *> items;
                // 设置每列的排序角色值
                QStandardItem *item0 = new QStandardItem(QString::number(++currentRowCount));
                item0->setData(currentRowCount, Qt::UserRole); // 设置为数值类型
                items.append(item0);

                QStandardItem *item1 = new QStandardItem(fileInfo.fileName());
                item1->setData(fileInfo.fileName(), Qt::UserRole); // 设置为字符串类型
                items.append(item1);

                QStandardItem *item2 = new QStandardItem(fileInfo.absoluteFilePath());
                item2->setData(fileInfo.absoluteFilePath(), Qt::UserRole); // 设置为字符串类型
                items.append(item2);

                QStandardItem *item3 = new QStandardItem(fileInfo.suffix());
                item3->setData(fileInfo.suffix(), Qt::UserRole); // 设置为字符串类型
                items.append(item3);

                QStandardItem *item4 = new QStandardItem(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
                item4->setData(fileInfo.birthTime(), Qt::UserRole); // 设置为日期时间类型
                items.append(item4);

                QStandardItem *item5 = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
                item5->setData(fileInfo.lastModified(), Qt::UserRole); // 设置为日期时间类型
                items.append(item5);

                tableModel->appendRow(items);
            }
        };
        QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
        firstSearch = false; // 设置为 false，表示已经进行了第一次搜索
    } else {
        if (++updateCounter % 500 == 0) {
            QVector<QString> filesBatchCopy = filesBatch;
            filesBatch.clear();

            auto updateUI = [this, filesBatchCopy]() {
                int currentRowCount = tableModel->rowCount();
                for (const QString &filePath : filesBatchCopy) {
                    QFileInfo fileInfo(filePath);
                    QList<QStandardItem *> items;
                    // 设置每列的排序角色值
                    QStandardItem *item0 = new QStandardItem(QString::number(++currentRowCount));
                    item0->setData(currentRowCount, Qt::UserRole); // 设置为数值类型
                    items.append(item0);

                    QStandardItem *item1 = new QStandardItem(fileInfo.fileName());
                    item1->setData(fileInfo.fileName(), Qt::UserRole); // 设置为字符串类型
                    items.append(item1);

                    QStandardItem *item2 = new QStandardItem(fileInfo.absoluteFilePath());
                    item2->setData(fileInfo.absoluteFilePath(), Qt::UserRole); // 设置为字符串类型
                    items.append(item2);

                    QStandardItem *item3 = new QStandardItem(fileInfo.suffix());
                    item3->setData(fileInfo.suffix(), Qt::UserRole); // 设置为字符串类型
                    items.append(item3);

                    QStandardItem *item4 = new QStandardItem(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
                    item4->setData(fileInfo.birthTime(), Qt::UserRole); // 设置为日期时间类型
                    items.append(item4);

                    QStandardItem *item5 = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
                    item5->setData(fileInfo.lastModified(), Qt::UserRole); // 设置为日期时间类型
                    items.append(item5);

                    tableModel->appendRow(items);
                }
            };
            QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
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
                // 设置每列的排序角色值
                QStandardItem *item0 = new QStandardItem(QString::number(++currentRowCount));
                item0->setData(currentRowCount, Qt::UserRole); // 设置为数值类型
                items.append(item0);

                QStandardItem *item1 = new QStandardItem(fileInfo.fileName());
                item1->setData(fileInfo.fileName(), Qt::UserRole); // 设置为字符串类型
                items.append(item1);

                QStandardItem *item2 = new QStandardItem(fileInfo.absoluteFilePath());
                item2->setData(fileInfo.absoluteFilePath(), Qt::UserRole); // 设置为字符串类型
                items.append(item2);

                QStandardItem *item3 = new QStandardItem(fileInfo.suffix());
                item3->setData(fileInfo.suffix(), Qt::UserRole); // 设置为字符串类型
                items.append(item3);

                QStandardItem *item4 = new QStandardItem(fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
                item4->setData(fileInfo.birthTime(), Qt::UserRole); // 设置为日期时间类型
                items.append(item4);

                QStandardItem *item5 = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
                item5->setData(fileInfo.lastModified(), Qt::UserRole); // 设置为日期时间类型
                items.append(item5);

                tableModel->appendRow(items);
            }
        };
        QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
    }

    // resultTableView->setSortingEnabled(true);
    // resultTableView->sortByColumn(0, Qt::AscendingOrder); // 按照序列号列排序
}

void FileSearch::onSearchFinished() {
    activeTaskCount--;
    progressBar->setValue(progressBar->value() + 1);
    updateProgressLabel();

    QObject *sender = QObject::sender();
    if (sender) {
        FileSearchThread *task = qobject_cast<FileSearchThread *>(sender);
        if (task) {
            activeTasks.removeAll(task); // 从活动任务列表中移除已完成的任务
            task->deleteLater();
        }
    }

    if (activeTaskCount == 0) {
        finishSearch(); // 确保处理剩余文件
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
    if(isStopping){
        QMessageBox::information(this, "搜索完成", QString("搜索耗时: %1 毫秒").arg(elapsedTime));
    }
    timer.invalidate();
    Logger::instance().log(QString("计时结束，搜索耗时: %1 毫秒").arg(elapsedTime));
}

void FileSearch::onFinishButtonClicked() {
    if (!isSearching) {
        Logger::instance().log("没有正在执行的搜索任务。");
        QMessageBox::information(this, "搜索中断", "没有正在执行的搜索任务。");
        return;
    }

    stopAllTasks();
    qint64 elapsedTime = timer.elapsed();
    if (!isStopping){
        QMessageBox::information(this, "搜索中断", QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    }
    timer.invalidate();
    Logger::instance().log(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));

    finishSearch();  // 调用finishSearch()方法处理并显示剩余文件

    progressBar->setValue(progressBar->maximum());
    updateProgressLabel();
    isSearching = false;
}

void FileSearch::stopAllTasks() {
    for (FileSearchThread *task : activeTasks) {
        task->stop();
    }
    activeTasks.clear();
    threadPool->clear();
}

void FileSearch::onSearchFilterChanged(const QString &text) {
    proxyModel->setFilterWildcard(text);
}
