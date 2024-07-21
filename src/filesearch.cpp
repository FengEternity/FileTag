#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include "FileSearchThread.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QElapsedTimer>

// 构造函数，初始化界面和成员变量
FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        threadPool(new QThreadPool(this)),
        timer(),
        updateCounter(0),
        activeTaskCount(0), // 初始化活动任务计数器
        totalDirectories(0), // 初始化目录总数
        isSearching(false) // 初始化搜索状态
{
    ui->setupUi(this);

    searchButton = ui->searchButton;
    searchLineEdit = ui->searchLineEdit;
    pathLineEdit = ui->pathLineEdit;
    resultListWidget = ui->resultListWidget;
    finishButton = ui->finishButton;
    progressBar = ui->progressBar;
    progressLabel = ui->progressLabel;

    connect(searchButton, &QPushButton::clicked, this, &FileSearch::onSearchButtonClicked);
    connect(finishButton, &QPushButton::clicked, this, &FileSearch::onFinishButtonClicked);

    Logger::instance().log("列表视图模型设置完成。");

    if (!layout()) {
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(resultListWidget);
        setLayout(layout);
    }

    threadPool->setMaxThreadCount(QThread::idealThreadCount()); // 根据系统的 CPU 核心数设置最大线程数
    Logger::instance().log("线程池初始化完成, 最大线程数: " + QString::number(threadPool->maxThreadCount()));
}

// 析构函数，释放资源
FileSearch::~FileSearch() {
    threadPool->waitForDone();
    delete ui;
}

// 搜索按钮点击事件
void FileSearch::onSearchButtonClicked() {
    QString searchKeyword = searchLineEdit->text();
    QString searchPath = pathLineEdit->text();

    if (searchPath.isEmpty()) {
        searchPath = QDir::rootPath();
    }

    resultListWidget->clear();
    Logger::instance().log("Search started for keyword: " + searchKeyword + " in path: " + searchPath);

    timer.start();
    Logger::instance().log("搜索计时开始。");
    activeTaskCount = 0;
    updateCounter = 0;
    totalDirectories = 0;
    isSearching = true; // 设置搜索状态为进行中

    // 第一次遍历计算目录总数
    QDirIterator dirIt(searchPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext()) {
        dirIt.next();
        totalDirectories++;
    }

    progressBar->setMaximum(totalDirectories);
    progressBar->setValue(0);
    updateProgressLabel();

    // 第二次遍历进行搜索
    QDirIterator it(searchPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QString dirPath = it.next();
        auto *task = new FileSearchThread(searchKeyword, dirPath);
        connect(task, &FileSearchThread::fileFound, this, &FileSearch::onFileFound);
        connect(task, &FileSearchThread::searchFinished, this, &FileSearch::onSearchFinished);
        activeTaskCount++;
        threadPool->start(task);
    }
}

// 搜索到文件时的槽函数
void FileSearch::onFileFound(const QString &filePath) {
    static QVector<QString> filesBatch;
    filesBatch.append(filePath);
    Logger::instance().log("文件找到: " + filePath);

    if(firstSearch) {
        QVector<QString> filesBatchCopy = filesBatch; // 创建一个副本
        filesBatch.clear(); // 清空原有的批量文件列表

        auto updateUI = [this, filesBatchCopy]() {
            resultListWidget->addItems(filesBatchCopy.toList());
            resultListWidget->reset();
            resultListWidget->update();
            resultListWidget->viewport()->update();
        };
        QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
    } else {
        if (++updateCounter % 500 == 0) { // 每找到500个文件更新一次
            QVector<QString> filesBatchCopy = filesBatch; // 创建一个副本
            filesBatch.clear(); // 清空原有的批量文件列表

            auto updateUI = [this, filesBatchCopy]() {
                resultListWidget->addItems(filesBatchCopy.toList());
                resultListWidget->reset();
                resultListWidget->update();
                resultListWidget->viewport()->update();
            };
            QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
        }
   }
//    if (++updateCounter % 500 == 0) { // 每找到500个文件更新一次
//        QVector<QString> filesBatchCopy = filesBatch; // 创建一个副本
//        filesBatch.clear(); // 清空原有的批量文件列表
//
//        auto updateUI = [this, filesBatchCopy]() {
//            resultListWidget->addItems(filesBatchCopy.toList());
//            resultListWidget->reset();
//            resultListWidget->update();
//            resultListWidget->viewport()->update();
//        };
//        QMetaObject::invokeMethod(this, updateUI, Qt::QueuedConnection);
//    }
}

// 搜索完成时的槽函数
void FileSearch::onSearchFinished() {
    activeTaskCount--;
    progressBar->setValue(progressBar->value() + 1); // 更新进度条
    updateProgressLabel();

    QObject *sender = QObject::sender();
    if (sender) {
        FileSearchThread *task = qobject_cast<FileSearchThread *>(sender);
        if (task) {
            task->deleteLater();
        }
    }

    if (activeTaskCount == 0) {
        qint64 elapsedTime = timer.elapsed();
        onSearchTime(elapsedTime);
        progressBar->setValue(totalDirectories); // 搜索完成后将进度条设为最大值
        updateProgressLabel();
        isSearching = false; // 搜索完成后重置搜索状态
    }
}

// 更新进度百分比标签
void FileSearch::updateProgressLabel() {
    int percentage = static_cast<int>((static_cast<float>(progressBar->value()) / totalDirectories) * 100);
    progressLabel->setText(QString::number(percentage) + "%");
}

// 搜索耗时的槽函数
void FileSearch::onSearchTime(qint64 elapsedTime) {
    QMessageBox::information(this, "搜索完成", QString("搜索耗时: %1 毫秒").arg(elapsedTime));
    timer.invalidate();
    Logger::instance().log(QString("计时结束，搜索耗时: %1 毫秒").arg(elapsedTime));
}

// 中断搜索按钮点击事件
void FileSearch::onFinishButtonClicked() {
    if (!isSearching) {
        Logger::instance().log("没有正在执行的搜索任务。");
        QMessageBox::information(this, "搜索中断", "没有正在执行的搜索任务。");
        return; // 如果没有正在执行的搜索任务，直接返回
    }

    threadPool->clear();
    qint64 elapsedTime = timer.elapsed();
    QMessageBox::information(this, "搜索中断", QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    timer.invalidate();
    Logger::instance().log(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    progressBar->setValue(progressBar->maximum()); // 搜索中断时将进度条设为最大值
    updateProgressLabel();
    isSearching = false; // 中断搜索后重置搜索状态
}
