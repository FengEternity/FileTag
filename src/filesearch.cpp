#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include "FileSearchThread.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QDebug>
#include <iostream>
#include <QCoreApplication>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QMessageBox> // 引入 QMessageBox 头文件
#include <QElapsedTimer> // 引入 QElapsedTimer

FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        resultModel(new CustomModel(this)),
        currentSearchThread(nullptr)  // 初始化指针
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
    if (currentSearchThread) {
        currentSearchThread->stop();
        currentSearchThread->wait();
        delete currentSearchThread;
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

    if (currentSearchThread) {
        currentSearchThread->stop();
        currentSearchThread->wait();
        delete currentSearchThread;
    }

    timer.start(); // 开始计时
    currentSearchThread = new FileSearchThread(searchKeyword, searchPath, this);
    connect(currentSearchThread, &FileSearchThread::fileFound, this, &FileSearch::onFileFound);
    connect(currentSearchThread, &FileSearchThread::searchFinished, this, &FileSearch::onSearchFinished);
    connect(currentSearchThread, &FileSearchThread::searchTime, this, &FileSearch::onSearchTime); // 连接 searchTime 信号和槽
    currentSearchThread->start();
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
    if (resultListWidget->count() > 0) {
        resultListWidget->scrollToItem(resultListWidget->item(0));
    }
    if (currentSearchThread) {
        currentSearchThread = nullptr;
    }
}

void FileSearch::onSearchTime(qint64 elapsedTime) {
    // 显示搜索耗时
    QMessageBox::information(this, "搜索完成", QString("搜索耗时: %1 毫秒").arg(elapsedTime));
    Logger::instance().log(QString("搜索耗时: %1 毫秒").arg(elapsedTime));
}

void FileSearch::onFinishButtonClicked() {
    if (currentSearchThread) {
        currentSearchThread->stop();
        currentSearchThread->wait();
        delete currentSearchThread;
        currentSearchThread = nullptr;

        // 计算并显示耗时
        qint64 elapsedTime = timer.elapsed();
        // QMessageBox::information(this, "搜索中断", QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
        Logger::instance().log(QString("搜索线程被中断，已耗时: %1 毫秒").arg(elapsedTime));
    }
}