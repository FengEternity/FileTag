/*
 * FileSearch.cpp
 * Author: Montee
 * CreateDate: 2024-11-1
 * Updater: Montee
 * UpdateDate: 2024-11-1
 * Summary: 文件搜索窗口类的实现文件
 */

#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QStandardItem>
#include <QCheckBox>

#include "Logger.h"
#include "FileSearch.h"
#include "ui_FileSearch.h"

/*
 * Summary: 构造函数，初始化UI和成员变量
 * Parameters:
 * QWidget *parent - 父窗口指针，默认值为 nullptr
 * Return: 无
 */
FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch),
        searchCore(new FileSearchCore(this))
{
    ui->setupUi(this);

    // 连接界面元素到成员变量
    searchButton = ui->searchButton;
    searchLineEdit = ui->searchLineEdit;
    pathLineEdit = ui->pathLineEdit;
    filterLineEdit = ui->filterLineEdit;
    resultTableView = ui->resultTableView;
    systemFilesCheckBox = ui->systemFilesCheckBox;
    systemFilesCheckBox->setChecked(false); // 默认不搜索系统文件

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

    // 连接 FileSearchCore 的信号到槽函数
    connect(searchCore, &FileSearchCore::fileFound, this, &FileSearch::onFileFound);
    connect(searchCore, &FileSearchCore::searchFinished, this, &FileSearch::onSearchFinished);
    connect(searchCore, &FileSearchCore::progressUpdated, this, &FileSearch::updateProgress);

    // 初始化文件数据库
    // searchCore->initFileDatabase();
}

/*
 * Summary: 析构函数，释放资源
 * Parameters: 无
 * Return: 无
 */
FileSearch::~FileSearch() {
    delete ui;
}

/*
 * Summary: 处理搜索按钮点击事件
 * Parameters: 无
 * Return: void
 */
void FileSearch::onSearchButtonClicked() {
    QString searchKeyword = searchLineEdit->text();
    QString searchPath = pathLineEdit->text();
    bool includeSystemFiles = systemFilesCheckBox->isChecked(); // 获取复选框状态

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

    searchCore->startSearch(searchKeyword, searchPath, includeSystemFiles);
}

/*
 * Summary: 处理停止按钮点击事件
 * Parameters: 无
 * Return: void
 */
void FileSearch::onFinishButtonClicked() {
    searchCore->stopSearch();
}

/*
 * Summary: 处理搜索过滤文本变化事件
 * Parameters:
 * const QString &text - 输入的过滤文本
 * Return: void
 */
void FileSearch::onSearchFilterChanged(const QString &text) {
    proxyModel->setFilterWildcard(text);
}

/*
 * Summary: 处理文件找到的信号，更新UI
 * Parameters:
 * const QString &filePath - 文件路径
 * Return: void
 */
void FileSearch::onFileFound(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QList<QStandardItem *> items;

    QStandardItem *item0 = new QStandardItem(QString::number(tableModel->rowCount() + 1));
    item0->setData(tableModel->rowCount() + 1, Qt::UserRole);
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

/*
 * Summary: 处理搜索完成的信号，通知用户
 * Parameters: 无
 * Return: void
 */
void FileSearch::onSearchFinished() {
    QMessageBox::information(this, "搜索完成", "文件搜索已完成。");
}

/*
 * Summary: 更新进度条和标签
 * Parameters:
 * int value - 当前进度值
 * int total - 总进度值
 * Return: void
 */
void FileSearch::updateProgress(int value, int total) {
    progressBar->setMaximum(total);
    progressBar->setValue(value);
    updateProgressLabel(value, total);
}

/*
 * Summary: 更新进度标签显示
 * Parameters:
 * int value - 当前进度值
 * int total - 总进度值
 * Return: void
 */
void FileSearch::updateProgressLabel(int value, int total) {
    int percentage = static_cast<int>((static_cast<float>(value) / total) * 100);
    progressLabel->setText(QString::number(percentage) + "%");
}
