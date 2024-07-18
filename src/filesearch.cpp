#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include "FileSearchThread.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QMessageBox>
#include <QElapsedTimer>

// 构造函数，初始化界面和成员变量
FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        threadPool(new QThreadPool(this)),
        timer(),
        updateCounter(0),
        activeTaskCount(0) // 初始化活动任务计数器
{
    ui->setupUi(this);

    searchButton = ui->searchButton;
    searchLineEdit = ui->searchLineEdit;
    pathLineEdit = ui->pathLineEdit;
    resultListWidget = ui->resultListWidget;
    finishButton = ui->finishButton;

    connect(searchButton, &QPushButton::clicked, this, &FileSearch::onSearchButtonClicked);
    connect(finishButton, &QPushButton::clicked, this, &FileSearch::onFinishButtonClicked);

    Logger::instance().log("列表视图模型设置完成。");

    if (!layout()) {
        auto *layout = new QVBoxLayout(this);
        layout->addWidget(resultListWidget);
        setLayout(layout);
    }

    threadPool->setMaxThreadCount(10); // 设置线程池最大线程数
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
    activeTaskCount = 0;

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
    Logger::instance().log("Found file: " + filePath);

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

// 搜索完成时的槽函数
void FileSearch::onSearchFinished() {
    activeTaskCount--;
    if (activeTaskCount == 0) {
        qint64 elapsedTime = timer.elapsed();
        onSearchTime(elapsedTime);
    }
}

// 搜索耗时的槽函数
void FileSearch::onSearchTime(qint64 elapsedTime) {
    QMessageBox::information(this, "搜索完成", QString("搜索耗时: %1 毫秒").arg(elapsedTime));
    Logger::instance().log(QString("搜索耗时: %1 毫秒").arg(elapsedTime));
}

// 中断搜索按钮点击事件
void FileSearch::onFinishButtonClicked() {
    threadPool->clear();
    qint64 elapsedTime = timer.elapsed();
    QMessageBox::information(this, "搜索中断", QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    Logger::instance().log(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
}
