#include "FileSearch.h"
#include "ui_FileSearch.h"
#include "Logger.h"
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QDebug>
#include <iostream>
#include <QCoreApplication>

FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        resultModel(new CustomModel(this))  // 使用自定义模型
{
    ui->setupUi(this);

    // 确保 UI 元素已经正确初始化
    searchButton = ui->searchButton;
    searchLineEdit = ui->searchLineEdit;
    pathLineEdit = ui->pathLineEdit;
    resultListWidget = ui->resultListWidget;

    // 连接信号和槽
    connect(searchButton, &QPushButton::clicked, this, &FileSearch::onSearchButtonClicked);

    // 初始化 resultListWidget
    qDebug() << "List view model set.";

    // 检查是否已有布局，避免重复设置
    if (!layout()) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(resultListWidget);
        setLayout(layout);
    }
}

FileSearch::~FileSearch() {
    delete ui;
}

void FileSearch::onSearchButtonClicked() {
    QString searchKeyword = searchLineEdit->text();
    QString searchPath = pathLineEdit->text();

    // 如果路径为空，则设置为根目录
    if (searchPath.isEmpty()) {
        searchPath = QDir::rootPath();
    }

    // 清空之前的搜索结果
    resultListWidget->clear();
    Logger::instance().log("Search started for keyword: " + searchKeyword + " in path: " + searchPath);

    // 强制刷新视图
    resultListWidget->reset();
    resultListWidget->update();
    resultListWidget->viewport()->update();
    QCoreApplication::processEvents(); // 强制处理所有挂起的事件

    // 进行文件搜索
    QDirIterator it(searchPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (filePath.contains(searchKeyword, Qt::CaseInsensitive)) {
            resultListWidget->addItem(filePath);
            Logger::instance().log("Found file: " + filePath);

            // 每次找到文件后刷新视图
            resultListWidget->reset();
            resultListWidget->update();
            resultListWidget->viewport()->update();
            QCoreApplication::processEvents(); // 强制处理所有挂起的事件
        }
    }

    // 搜索完成后，滚动到第一个结果项
    if (resultListWidget->count() > 0) {
        resultListWidget->scrollToItem(resultListWidget->item(0));
    }
}
