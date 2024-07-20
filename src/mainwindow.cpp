#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MultiSelectDialog.h"
#include "FileSearch.h"
#include "Logger.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QCloseEvent>
#include <QStringListModel>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          ui(new Ui::MainWindow),
          fileModel(new QFileSystemModel(this)),
          fileTagSystem("tags.csv", "users.csv") {

    ui->setupUi(this);

    setWindowTitle("FileTAG");
    resize(600, 400);

    // 设置工具栏位置
    // addToolBar(Qt::LeftToolBarArea, ui->toolBar);

    // 设置菜单和工具栏动作
    connect(ui->actionFileSearch, &QAction::triggered, this, &MainWindow::onFileSearchClicked);
    connect(ui->actionFileTransfer, &QAction::triggered, this, &MainWindow::onFileTransferClicked);
    connect(ui->actionAddTag, &QAction::triggered, this, &MainWindow::onAddTagClicked);
    connect(ui->actionSearchTag, &QAction::triggered, this, &MainWindow::onSearchTagClicked);
    connect(ui->actionRemoveTag, &QAction::triggered, this, &MainWindow::onRemoveTagClicked);
    connect(ui->actionUpdateTag, &QAction::triggered, this, &MainWindow::onUpdateTagClicked);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->actionDocumentation, &QAction::triggered, this, &MainWindow::showDocumentation);

    // 设置文件系统模型和视图
    fileModel->setRootPath(QDir::currentPath());
    ui->fileView->setModel(fileModel);
    ui->fileView->setRootIndex(fileModel->index(QDir::currentPath()));
    ui->fileView->setViewMode(QListView::IconMode);
    ui->fileView->setIconSize(QSize(64, 64));

    initializeView(); // 初始化为空视图
    populateTags(); // 填充标签列表

    // 连接信号和槽
    connect(ui->tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);
    connect(ui->fileView, &QListView::clicked, this, &MainWindow::onFileClicked);

    // 读取并应用样式表
    QFile file(":/stylesheet.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QTextStream(&file).readAll();
        setStyleSheet(styleSheet);
        file.close();
    }

    // 设置 spliter，使左侧区域只占 30%
    QList<int> sizes;
    sizes << 100 << 420;
    ui->splitter->setSizes(sizes);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    emit mainWindowClosed();
    QMainWindow::closeEvent(event);
}

void MainWindow::onFileActionClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*)");
    if (!filePath.isEmpty()) {
        QMessageBox::information(this, "文件已选择", "您选择的文件是: " + filePath);
    }
}

void MainWindow::onFileSearchClicked() {
    QWidget *currentCentralWidget = takeCentralWidget();
    if (currentCentralWidget) {
        delete currentCentralWidget;
    }
    FileSearch *fileSearch = new FileSearch(this);
    setCentralWidget(fileSearch);
}

void MainWindow::onFileTransferClicked() {
    // 文件传输功能的空实现
}

void MainWindow::initializeView() {
    QStringListModel *emptyModel = new QStringListModel(this);
    ui->fileView->setModel(emptyModel);
}

void MainWindow::onAddTagClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*)");
    if (filePath.isEmpty()) {
        return;
    }
    QString tag = QInputDialog::getText(this, "添加标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        fileTagSystem.addTags(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已添加", "标签已添加到文件: " + filePath);
        Logger::instance().log("标签已添加到文件: " + filePath);
        populateTags();
    }
}

void MainWindow::onSearchTagClicked() {
    QString tag = QInputDialog::getText(this, "搜索标签", "请输入标签:");
    if (!tag.isEmpty()) {
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag.toStdString());
        QStringList fileList;
        for (const auto &file : files) {
            fileList.append(QString::fromStdString(file));
        }
        displayFiles(fileList);  // 显示文件列表
    }
}

void MainWindow::onRemoveTagClicked() {
    QString tag = QInputDialog::getText(this, "删除标签", "请输入标签:");
    if (!tag.isEmpty()) {
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag.toStdString());
        if (files.empty()) {
            QMessageBox::information(this, "无文件", "没有文件包含此标签。");
            Logger::instance().log("没有文件包含此标签。");
            return;
        }
        QStringList fileList;
        for (const auto &file : files) {
            fileList.append(QString::fromStdString(file));
        }
        MultiSelectDialog dialog(fileList, this);
        if (dialog.exec() == QDialog::Accepted) {
            QStringList selectedFiles = dialog.selectedItems();
            if (selectedFiles.contains("删除所有文件")) {
                for (const auto &file : files) {
                    fileTagSystem.removeTag(file, tag.toStdString());
                }
                QMessageBox::information(this, "标签已删除", "标签已从所有文件删除。");
                Logger::instance().log("标签已从所有文件删除。");
            } else {
                for (const auto &selectedFile : selectedFiles) {
                    fileTagSystem.removeTag(selectedFile.toStdString(), tag.toStdString());
                }
                QMessageBox::information(this, "标签已删除", "标签已从选中的文件中删除。");
                Logger::instance().log("标签已从选中的文件中删除。");
            }
            populateTags();
        }
    }
}

void MainWindow::onUpdateTagClicked() {
    QString filePath = QInputDialog::getText(this, "更新标签", "请输入文件路径:");
    QString oldTag = QInputDialog::getText(this, "更新标签", "请输入旧标签:");
    QString newTag = QInputDialog::getText(this, "更新标签", "请输入新标签:");

    if (!filePath.isEmpty() && !oldTag.isEmpty() && !newTag.isEmpty()) {
        fileTagSystem.updateTag(filePath.toStdString(), oldTag.toStdString(), newTag.toStdString());
        QMessageBox::information(this, "标签已更新", "文件中的标签已更新: " + filePath);
        Logger::instance().log("文件中的标签已更新: " + filePath);
        populateTags();
    }
}

void MainWindow::onTagSelected() {
    QListWidgetItem *item = ui->tagListWidget->currentItem();
    if (item) {
        std::string tag = item->text().toStdString();
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag);
        QStringList fileList;
        for (const auto &file : files) {
            fileList.append(QString::fromStdString(file));
        }
        displayFiles(fileList);  // 显示文件列表
    }
}

void MainWindow::populateTags() {
    ui->tagListWidget->clear();
    std::vector<std::string> tags = fileTagSystem.listAllTags();
    for (const auto &tag : tags) {
        ui->tagListWidget->addItem(QString::fromStdString(tag));
    }
}

void MainWindow::displayFiles(const QStringList &filepaths) {
    Logger::instance().log("displayFiles 调用参数：" + filepaths.join(", "));

    if (filepaths.isEmpty()) {
        QStringListModel *emptyModel = new QStringListModel(this);
        ui->fileView->setModel(emptyModel);
        ui->fileView->setRootIndex(QModelIndex());
        Logger::instance().log("视图已清除");
        return;
    }
    QFileInfo firstFile(filepaths.first());
    QString directory = firstFile.absolutePath();

    ui->fileView->setModel(fileModel);
    fileModel->setRootPath(directory);
    ui->fileView->setRootIndex(fileModel->index(directory));
    Logger::instance().log("设置根目录为：" + directory);

    QStringList nameFilters;
    for (const QString &filePath : filepaths) {
        QFileInfo fileInfo(filePath);
        nameFilters << fileInfo.fileName();
        Logger::instance().log("添加过滤文件：" + fileInfo.fileName());
    }
    fileModel->setNameFilters(nameFilters);
    fileModel->setNameFilterDisables(false);
    ui->fileView->setRootIndex(fileModel->index(directory));

    for (int i = 0; i < fileModel->rowCount(ui->fileView->rootIndex()); ++i) {
        QModelIndex index = fileModel->index(i, 0, ui->fileView->rootIndex());
        QString fileName = fileModel->fileName(index);
        if (fileName.length() > 20) {
            QString shortName = fileName.left(17) + "...";
            fileModel->setData(index, shortName, Qt::DisplayRole);
            Logger::instance().log("截断文件名：" + fileName + "为：" + shortName);
        }
    }
    ui->fileView->update();
}

void MainWindow::onFileClicked(const QModelIndex &index) {
    QString filePath = fileModel->filePath(index);
    showFilePreview(filePath);
}

void MainWindow::showFilePreview(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (fileInfo.isFile()) {
        if (fileInfo.suffix().toLower() == "txt" || fileInfo.suffix().toLower() == "md") {
            QFile file(filePath);
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&file);
                QString content = in.readAll();
                file.close();

                QTextEdit *textEdit = new QTextEdit;
                textEdit->setReadOnly(true);
                textEdit->setPlainText(content);

                QDialog *dialog = new QDialog(this);
                dialog->setWindowTitle(fileInfo.fileName());
                QVBoxLayout *layout = new QVBoxLayout;
                layout->addWidget(textEdit);
                dialog->setLayout(layout);
                dialog->exec();
            }
        } else if (fileInfo.suffix().toLower() == "png" || fileInfo.suffix().toLower() == "jpg" || fileInfo.suffix().toLower() == "jpeg") {
            QLabel *imageLabel = new QLabel;
            QPixmap pixmap(filePath);
            imageLabel->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio));

            QDialog *dialog = new QDialog(this);
            dialog->setWindowTitle(fileInfo.fileName());
            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(imageLabel);
            dialog->setLayout(layout);
            dialog->exec();
        }
    }
}

void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "关于", "FileTag 是一个简单的文件标签工具。\n"
                                     "版本：1.0\n"
                                     "作者：Montee");
}

void MainWindow::showDocumentation() {
    QMessageBox::information(this, "帮助文档", "使用过程中如遇到问题欢迎联系我。\n"
                                               "邮件：2605958732@qq.com");
}
