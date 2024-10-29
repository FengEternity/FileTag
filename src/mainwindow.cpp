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
          fileTagSystem("tags.csv", "users.csv"),
          homeWidget(nullptr) {

    ui->setupUi(this); // 确保 setupUi 被正确调用

    // 初始化 homeWidget
    homeWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(homeWidget);
    layout->addWidget(ui->splitter);  // 将 splitter 添加到 homeWidget

    setCentralWidget(homeWidget);  // 设置 homeWidget 为中央控件

    setWindowTitle("FileTAG");
    resize(600, 400);

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

// 关闭事件处理
void MainWindow::closeEvent(QCloseEvent *event) {
    emit mainWindowClosed();
    QMainWindow::closeEvent(event);
}

// 文件操作按钮点击事件处理
void MainWindow::onFileActionClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*)");
    if (!filePath.isEmpty()) {
        QMessageBox::information(this, "文件已选择", "您选择的文件是: " + filePath);
    }
}

// 文件搜索按钮点击事件处理
void MainWindow::onFileSearchClicked() {
    QWidget *currentCentralWidget = takeCentralWidget();
    if (currentCentralWidget && currentCentralWidget != homeWidget) {
        delete currentCentralWidget;
    }
    FileSearch *fileSearch = new FileSearch(this);
    setCentralWidget(fileSearch);
}

// 文件传输按钮点击事件处理
// 文件传输按钮点击事件处理
void MainWindow::onFileTransferClicked() {
    QWidget *currentCentralWidget = takeCentralWidget();
    if (currentCentralWidget && currentCentralWidget!= homeWidget) {
        delete currentCentralWidget;
    }

    FileTransfer *fileTransfer = new FileTransfer(this);

    // 连接文件传输对话框中的信号到MainWindow中的槽函数（假设FileTransfer类中有相关信号）
    connect(fileTransfer, &FileTransfer::fileSelected, this, &MainWindow::onFileSelectedInTransfer);
    connect(fileTransfer, &FileTransfer::fileTransferProgress, this, &MainWindow::onFileTransferProgress);
    connect(fileTransfer, &FileTransfer::fileTransferFinished, this, &MainWindow::onFileTransferFinished);

    setCentralWidget(fileTransfer);
}

// 当在文件传输对话框中选择文件完成后调用此槽函数
void MainWindow::onFileSelectedInTransfer(const QString& filePath) {
    // 可以在这里进行一些与选择文件完成相关的操作，比如在界面上显示选择的文件路径等
    qDebug() << "在文件传输对话框中选择的文件路径为：" << filePath;
}

// 当文件传输过程中有进度更新时调用此槽函数
void MainWindow::onFileTransferProgress(qint64 bytesSent, qint64 totalBytes) {
    // 计算传输进度百分比
    double progress = static_cast<double>(bytesSent) / static_cast<double>(totalBytes) * 100.0;

    // 可以在这里进行一些与更新传输进度相关的操作，比如在界面上更新进度条的值等
    qDebug() << "文件传输进度：" << progress << "%";
}

// 当文件传输完成后调用此槽函数
void MainWindow::onFileTransferFinished(bool success) {
    if (success) {
        qDebug() << "文件传输成功！";
        // 可以在这里进行一些与文件传输成功相关的操作，比如弹出提示框告知用户等
    } else {
        qDebug() << "文件传输失败！";
        // 可以在这里进行一些与文件传输失败相关的操作，比如弹出错误提示框告知用户失败原因等
    }
}


// 初始化视图
void MainWindow::initializeView() {
    QStringListModel *emptyModel = new QStringListModel(this);
    ui->fileView->setModel(emptyModel);
}

// 添加标签按钮点击事件处理
void MainWindow::onAddTagClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*)");
    if (filePath.isEmpty()) {
        return;
    }
    QString tag = QInputDialog::getText(this, "添加标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        fileTagSystem.addTags(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已添加", "标签已添加到文件: " + filePath);
        LOG_INFO("标签已添加到文件: " + filePath);
        populateTags();
    }
}

// 搜索标签按钮点击事件处理
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

// 删除标签按钮点击事件处理
void MainWindow::onRemoveTagClicked() {
    QString tag = QInputDialog::getText(this, "删除标签", "请输入标签:");
    if (!tag.isEmpty()) {
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag.toStdString());
        if (files.empty()) {
            QMessageBox::information(this, "无文件", "没有文件包含此标签。");
            LOG_INFO("没有文件包含此标签。");
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
                LOG_INFO("标签已从所有文件删除。");
            } else {
                for (const auto &selectedFile : selectedFiles) {
                    fileTagSystem.removeTag(selectedFile.toStdString(), tag.toStdString());
                }
                QMessageBox::information(this, "标签已删除", "标签已从选中的文件中删除。");
                LOG_INFO("标签已从选中的文件中删除。");
            }
            populateTags();
        }
    }
}

// 更新标签按钮点击事件处理
void MainWindow::onUpdateTagClicked() {
    QString filePath = QInputDialog::getText(this, "更新标签", "请输入文件路径:");
    QString oldTag = QInputDialog::getText(this, "更新标签", "请输入旧标签:");
    QString newTag = QInputDialog::getText(this, "更新标签", "请输入新标签:");

    if (!filePath.isEmpty() && !oldTag.isEmpty() && !newTag.isEmpty()) {
        fileTagSystem.updateTag(filePath.toStdString(), oldTag.toStdString(), newTag.toStdString());
        QMessageBox::information(this, "标签已更新", "文件中的标签已更新: " + filePath);
        LOG_INFO("文件中的标签已更新: " + filePath);
        populateTags();
    }
}

// 标签列表项被选中事件处理
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

// 填充标签列表
void MainWindow::populateTags() {
    ui->tagListWidget->clear();
    std::vector<std::string> tags = fileTagSystem.listAllTags();
    for (const auto &tag : tags) {
        ui->tagListWidget->addItem(QString::fromStdString(tag));
    }
}

// 显示文件列表
void MainWindow::displayFiles(const QStringList &filepaths) {
    LOG_INFO("displayFiles 调用参数：" + filepaths.join(", "));

    if (filepaths.isEmpty()) {
        QStringListModel *emptyModel = new QStringListModel(this);
        ui->fileView->setModel(emptyModel);
        ui->fileView->setRootIndex(QModelIndex());
        LOG_INFO("视图已清除");
        return;
    }
    QFileInfo firstFile(filepaths.first());
    QString directory = firstFile.absolutePath();

    ui->fileView->setModel(fileModel);
    fileModel->setRootPath(directory);
    ui->fileView->setRootIndex(fileModel->index(directory));
    LOG_INFO("设置根目录为：" + directory);

    QStringList nameFilters;
    for (const QString &filePath : filepaths) {
        QFileInfo fileInfo(filePath);
        nameFilters << fileInfo.fileName();
        LOG_INFO("添加过滤文件：" + fileInfo.fileName());
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
            LOG_INFO("截断文件名：" + fileName + "为：" + shortName);
        }
    }
    ui->fileView->update();
}

// 文件视图中的文件被点击事件处理
void MainWindow::onFileClicked(const QModelIndex &index) {
    QString filePath = fileModel->filePath(index);
    showFilePreview(filePath);
}

// 显示文件预览
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

// 显示关于对话框
void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "关于", "FileTag 是一个简单的文件标签工具。\n"
                                     "版本：1.0\n"
                                     "作者：Montee\n"
                                     "协作者：Cranzz");
}

// 显示帮助文档
void MainWindow::showDocumentation() {
    QMessageBox::information(this, "帮助文档", "使用过程中如遇到问题欢迎联系我。\n"
                                               "邮件：2605958732@qq.com");
}

// 回到主窗口
void MainWindow::on_actionHome_triggered() {
    // 检查 homeWidget 指针是否有效
    if (!homeWidget) {
        qDebug() << "homeWidget 指针为空！";
        return;
    }

    setCentralWidget(homeWidget);

    // 清空文件视图
    initializeView();

    // 重新连接信号和槽
    connect(ui->tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);
    connect(ui->fileView, &QListView::clicked, this, &MainWindow::onFileClicked);

    populateTags(); // 填充标签列表
}