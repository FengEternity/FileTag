#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include "FileSearchThread.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QWaitCondition>
#include <QMessageBox> // 引入 QMessageBox 头文件
#include <QElapsedTimer> // 引入 QElapsedTimer

FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        resultModel(new CustomModel(this)),
        timer(), // 初始化计时器
        searchThreads() // 初始化搜索线程向量
{
    ui->setupUi(this);

    // 确保 UI 元素已经正确初始化
    searchButton = ui->searchButton;
    searchLineEdit = ui->searchLineEdit;
    pathLineEdit = ui->pathLineEdit;
    resultListWidget = ui->resultListWidget;
    finishButton = ui->finishButton;  // 初始化 finishButton

    // 连接信号和槽
    connect(searchButton, &QPushButton::clicked, this, &FileSearch::onSearchButtonClicked);
    connect(finishButton, &QPushButton::clicked, this, &FileSearch::onFinishButtonClicked);  // 连接信号和槽

    Logger::instance().log("List view model set.");

    if (!layout()) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(resultListWidget);
        setLayout(layout);
    }
}

FileSearch::~FileSearch() {
    for (auto thread : searchThreads) {
        if (thread) {
            thread->stop();
            thread->wait();
            delete thread;
        }
    }
    delete ui;
}

void FileSearch::onSearchButtonClicked() {
    QString searchKeyword = searchLineEdit->text();
    QString searchPath = pathLineEdit->text();

    if (searchPath.isEmpty()) {
        searchPath = QDir::rootPath();
    }

    resultListWidget->clear();
    Logger::instance().log("Search started for keyword: " + searchKeyword + " in path: " + searchPath);

    for (auto thread : searchThreads) {
        if (thread) {
            thread->stop();
            thread->wait();
            delete thread;
        }
    }
    searchThreads.clear();

    timer.start(); // 开始计时

    QDirIterator it(searchPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QString dirPath = it.next();
        FileSearchThread* thread = new FileSearchThread(searchKeyword, dirPath, this);
        connect(thread, &FileSearchThread::fileFound, this, &FileSearch::onFileFound);
        connect(thread, &FileSearchThread::searchFinished, this, &FileSearch::onSearchFinished);
        connect(thread, &FileSearchThread::searchTime, this, &FileSearch::onSearchTime); // 连接 searchTime 信号和槽
        searchThreads.append(thread);
        thread->start();
    }
}

void FileSearch::onFileFound(const QString &filePath) {
    resultListWidget->addItem(filePath);
    Logger::instance().log("Found file: " + filePath);

    resultListWidget->reset();
    resultListWidget->update();
    resultListWidget->viewport()->update();
    QCoreApplication::processEvents(); // 强制处理所有挂起的事件
}

void FileSearch::onSearchFinished() {
    bool allFinished = true;
    for (auto thread : searchThreads) {
        if (thread && thread->isRunning()) {
            allFinished = false;
            break;
        }
    }
    if (allFinished) {
        if (resultListWidget->count() > 0) {
            resultListWidget->scrollToItem(resultListWidget->item(0));
        }
        searchThreads.clear();
    }
}

void FileSearch::onSearchTime(qint64 elapsedTime) {
    // 显示搜索耗时
    QMessageBox::information(this, "搜索完成", QString("搜索耗时: %1 毫秒").arg(elapsedTime));
    Logger::instance().log(QString("搜索耗时: %1 毫秒").arg(elapsedTime));
}

void FileSearch::onFinishButtonClicked() {
    for (auto thread : searchThreads) {
        if (thread) {
            thread->stop();
            thread->wait();
            delete thread;
        }
    }
    searchThreads.clear();

    // 计算并显示耗时
    qint64 elapsedTime = timer.elapsed();
    QMessageBox::information(this, "搜索中断", QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    // Logger::instance().log(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
}