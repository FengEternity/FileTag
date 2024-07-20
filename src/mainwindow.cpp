#include "mainwindow.h"
#include "MultiSelectDialog.h"
#include "FileSearch.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListView>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QToolBar>
#include <QMenuBar>
#include <QFile>
#include <QTextStream>
#include <QImageReader>
#include <QTextEdit>
#include <QFileDialog>
#include <QToolButton>
#include <QMenu>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          centralWidget(new QWidget(this)),
          mainLayout(new QHBoxLayout),
          sideBarLayout(new QVBoxLayout),
          contentLayout(new QVBoxLayout),
          tagListWidget(new QListWidget(this)),
          fileView(new QListView(this)),
          fileModel(new QFileSystemModel(this)),
          fileTagSystem("tags.csv", "users.csv") {

    setWindowTitle("FileTag");
    resize(600, 400);

    // 创建菜单栏
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // 创建文件菜单和动作
    fileMenu = menuBar->addMenu(tr("文件"));
    fileSearchAction = new QAction(tr("文件搜索"), this);
    fileTransferAction = new QAction(tr("文件传输"), this);
    fileMenu->addAction(fileSearchAction);
    fileMenu->addAction(fileTransferAction);

    // 连接文件菜单动作的信号和槽
    connect(fileSearchAction, &QAction::triggered, this, &MainWindow::onFileSearchClicked);
    connect(fileTransferAction, &QAction::triggered, this, &MainWindow::onFileTransferClicked);

    // 创建标签菜单和动作
    tagMenu = menuBar->addMenu(tr("标签"));
    QAction *addTagAction = new QAction(tr("添加"), this);
    QAction *searchTagAction = new QAction(tr("搜索"), this);
    QAction *removeTagAction = new QAction(tr("删除"), this);
    QAction *updateTagAction = new QAction(tr("更新"), this);
    tagMenu->addAction(addTagAction);
    tagMenu->addAction(searchTagAction);
    tagMenu->addAction(removeTagAction);
    tagMenu->addAction(updateTagAction);

    // 创建工具栏和工具按钮
    toolBar = new QToolBar(this);
    addToolBar(Qt::TopToolBarArea, toolBar);

    // 文件工具按钮及其菜单
    QToolButton *fileToolButton = new QToolButton(this);
    fileToolButton->setText(tr("文件"));
    fileToolButton->setPopupMode(QToolButton::InstantPopup);
    QMenu *fileToolMenu = new QMenu(fileToolButton);
    fileToolMenu->addAction(fileSearchAction);
    fileToolMenu->addAction(fileTransferAction);
    fileToolButton->setMenu(fileToolMenu);
    toolBar->addWidget(fileToolButton);

    // 标签工具按钮及其菜单
    QToolButton *tagToolButton = new QToolButton(this);
    tagToolButton->setText(tr("标签"));
    tagToolButton->setPopupMode(QToolButton::InstantPopup);
    QMenu *tagToolMenu = new QMenu(tagToolButton);
    tagToolMenu->addAction(addTagAction);
    tagToolMenu->addAction(searchTagAction);
    tagToolMenu->addAction(removeTagAction);
    tagToolMenu->addAction(updateTagAction);
    tagToolButton->setMenu(tagToolMenu);
    toolBar->addWidget(tagToolButton);

    // 创建帮助菜单和动作
    helpMenu = menuBar->addMenu(tr("帮助"));
    QAction *aboutAction = new QAction(tr("关于"), this);
    QAction *documentationAction = new QAction(tr("文档"), this);
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(documentationAction);

    // 创建分割器并添加标签列表和文件视图
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tagListWidget);
    splitter->addWidget(fileView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);
    mainLayout->addWidget(splitter);

    // 设置中央窗口部件的布局
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 读取并应用样式表
    QFile file(":/stylesheet.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QTextStream(&file).readAll();
        setStyleSheet(styleSheet);
        file.close();
    }

    // 配置文件系统模型和文件视图
    fileModel->setRootPath(QDir::currentPath());
    fileView->setModel(fileModel);
    fileView->setRootIndex(fileModel->index(QDir::currentPath()));
    fileView->setViewMode(QListView::IconMode);
    fileView->setIconSize(QSize(64, 64));

    initializeView(); // 初始化为空视图
    populateTags(); // 填充标签列表

    // 连接信号和槽
    connect(tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);
    connect(fileView, &QListView::clicked, this, &MainWindow::onFileClicked);
    connect(addTagAction, &QAction::triggered, this, &MainWindow::onAddTagClicked);
    connect(searchTagAction, &QAction::triggered, this, &MainWindow::onSearchTagClicked);
    connect(removeTagAction, &QAction::triggered, this, &MainWindow::onRemoveTagClicked);
    connect(updateTagAction, &QAction::triggered, this, &MainWindow::onUpdateTagClicked);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(documentationAction, &QAction::triggered, this, &MainWindow::showDocumentation);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    emit mainWindowClosed();
    QMainWindow::closeEvent(event);
}

// 文件动作的槽函数实现
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

MainWindow::~MainWindow() {}

void MainWindow::initializeView() {
    QStringListModel *emptyModel = new QStringListModel(this);
    fileView->setModel(emptyModel);
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
    QListWidgetItem *item = tagListWidget->currentItem();
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
    tagListWidget->clear();
    std::vector<std::string> tags = fileTagSystem.listAllTags();
    for (const auto &tag : tags) {
        tagListWidget->addItem(QString::fromStdString(tag));
    }
}

void MainWindow::displayFiles(const QStringList &filepaths) {
    Logger::instance().log("displayFiles 调用参数：" + filepaths.join(", "));

    if (filepaths.isEmpty()) {
        QStringListModel *emptyModel = new QStringListModel(this);
        fileView->setModel(emptyModel);
        fileView->setRootIndex(QModelIndex());
        Logger::instance().log("视图已清除");
        return;
    }
    QFileInfo firstFile(filepaths.first());
    QString directory = firstFile.absolutePath();

    fileView->setModel(fileModel);
    fileModel->setRootPath(directory);
    fileView->setRootIndex(fileModel->index(directory));
    Logger::instance().log("设置根目录为：" + directory);

    QStringList nameFilters;
    for (const QString &filePath : filepaths) {
        QFileInfo fileInfo(filePath);
        nameFilters << fileInfo.fileName();
        Logger::instance().log("添加过滤文件：" + fileInfo.fileName());
    }
    fileModel->setNameFilters(nameFilters);
    fileModel->setNameFilterDisables(false);
    fileView->setRootIndex(fileModel->index(directory));

    for (int i = 0; i < fileModel->rowCount(fileView->rootIndex()); ++i) {
        QModelIndex index = fileModel->index(i, 0, fileView->rootIndex());
        QString fileName = fileModel->fileName(index);
        if (fileName.length() > 20) {
            QString shortName = fileName.left(17) + "...";
            fileModel->setData(index, shortName, Qt::DisplayRole);
            Logger::instance().log("截断文件名：" + fileName + "为：" + shortName);
        }
    }
    fileView->update();
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
