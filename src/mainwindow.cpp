#include "mainwindow.h"
#include "MultiSelectDialog.h"
#include "FileProcessor.h"
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
#include <QProgressBar>
#include <QDebug> // 添加调试信息

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          centralWidget(new QWidget(this)),
          mainLayout(new QHBoxLayout),
          sideBarLayout(new QVBoxLayout),
          contentLayout(new QVBoxLayout),
          tagListWidget(new QListWidget(this)),
          fileView(new QListView(this)),
          fileModel(new QFileSystemModel(this)),
          fileTagSystem("tags.csv", "users.csv"),
          fileProcessor(new FileProcessor(this)), // 初始化文件处理类
          progressBar(new QProgressBar(this)) { // 初始化进度条

    setWindowTitle("FileTag");
    resize(600, 400);

    // 创建菜单栏并添加到窗口
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // 创建文件菜单
    fileMenu = menuBar->addMenu(tr("文件"));

    // 创建标签菜单并添加相关动作
    tagMenu = menuBar->addMenu(tr("标签"));
    tagMenu->addAction(tr("添加标签"), this, &MainWindow::onAddTagClicked);
    tagMenu->addAction(tr("搜索标签"), this, &MainWindow::onSearchTagClicked);
    tagMenu->addAction(tr("删除标签"), this, &MainWindow::onRemoveTagClicked);
    tagMenu->addAction(tr("更新标签"), this, &MainWindow::onUpdateTagClicked);

    QAction *addTagAction = new QAction("添加标签", this);
    QAction *searchTagAction = new QAction("搜索标签", this);
    QAction *removeTagAction = new QAction("删除标签", this);
    QAction *updateTagAction = new QAction("更新标签", this);

    toolBar = new QToolBar(this);

    // 创建工具按钮
    QToolButton *toolButton = new QToolButton(this);
    toolButton->setText("标签");
    toolButton->setPopupMode(QToolButton::InstantPopup);

    // 创建标签菜单并添加到工具按钮
    QMenu *tagMenu = new QMenu(toolButton);
    tagMenu->addAction(addTagAction);
    tagMenu->addAction(searchTagAction);
    tagMenu->addAction(removeTagAction);
    tagMenu->addAction(updateTagAction);
    toolButton->setMenu(tagMenu);

    // 将工具按钮添加到工具栏
    toolBar->addWidget(toolButton);

    // 添加新的文件动作
    QAction *fileAction = new QAction("文件", this);
    toolBar->addAction(fileAction);

    // 将工具栏添加到窗口的顶部工具栏区域
    addToolBar(Qt::TopToolBarArea, toolBar);

    // 创建帮助菜单并添加相关动作
    helpMenu = menuBar->addMenu(tr("帮助"));
    QAction *aboutAction = new QAction(tr("关于"), this);
    QAction *documentationAction = new QAction(tr("文档"), this);
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(documentationAction);

    // 创建分割器并添加标签列表和文件视图
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tagListWidget);
    splitter->addWidget(fileView);

    // 设置分割器的伸缩因子
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    // 将分割器添加到主布局
    mainLayout->addWidget(splitter);

    // 设置中央窗口部件的布局
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 读取并应用样式表
    QFile file(":/stylesheet.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QTextStream(&file).readAll();
        this->setStyleSheet(styleSheet);
        qDebug() << "Stylesheet loaded successfully.";
        file.close();
    } else {
        qDebug() << "Could not open stylesheet file.";
    }

    // 配置文件系统模型和文件视图
    fileModel->setRootPath(QDir::currentPath());
    fileView->setModel(fileModel);
    fileView->setRootIndex(fileModel->index(QDir::currentPath()));
    fileView->setViewMode(QListView::IconMode); // 设置文件视图为图标模式
    fileView->setIconSize(QSize(64, 64)); // 设置图标大小

    initializeView(); // 初始化为空视图

    populateTags(); // 填充标签列表

    // 创建并添加文件处理相关的UI组件
    QVBoxLayout *fileProcessorLayout = new QVBoxLayout;
    QLineEdit *searchDirEdit = new QLineEdit(this);
    QLineEdit *searchKeywordEdit = new QLineEdit(this);
    QPushButton *searchButton = new QPushButton("搜索文件", this);
    QListWidget *resultList = new QListWidget(this);
    QLineEdit *uploadFilePathEdit = new QLineEdit(this);
    QPushButton *uploadButton = new QPushButton("上传文件", this);
    QLineEdit *downloadUrlEdit = new QLineEdit(this);
    QLineEdit *downloadSavePathEdit = new QLineEdit(this);
    QPushButton *downloadButton = new QPushButton("下载文件", this);

    fileProcessorLayout->addWidget(new QLabel("搜索目录:", this));
    fileProcessorLayout->addWidget(searchDirEdit);
    fileProcessorLayout->addWidget(new QLabel("搜索关键字:", this));
    fileProcessorLayout->addWidget(searchKeywordEdit);
    fileProcessorLayout->addWidget(searchButton);
    fileProcessorLayout->addWidget(new QLabel("搜索结果:", this));
    fileProcessorLayout->addWidget(resultList);
    fileProcessorLayout->addWidget(new QLabel("上传文件路径:", this));
    fileProcessorLayout->addWidget(uploadFilePathEdit);
    fileProcessorLayout->addWidget(uploadButton);
    fileProcessorLayout->addWidget(new QLabel("下载文件URL:", this));
    fileProcessorLayout->addWidget(downloadUrlEdit);
    fileProcessorLayout->addWidget(new QLabel("下载保存路径:", this));
    fileProcessorLayout->addWidget(downloadSavePathEdit);
    fileProcessorLayout->addWidget(downloadButton);
    fileProcessorLayout->addWidget(progressBar);

    sideBarLayout->addLayout(fileProcessorLayout);
    mainLayout->addLayout(sideBarLayout);

    // 连接信号和槽
    connect(tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);
    connect(fileView, &QListView::clicked, this, &MainWindow::onFileClicked);

    // 连接标签相关动作的信号和槽
    connect(addTagAction, &QAction::triggered, this, &MainWindow::onAddTagClicked);
    connect(searchTagAction, &QAction::triggered, this, &MainWindow::onSearchTagClicked);
    connect(removeTagAction, &QAction::triggered, this, &MainWindow::onRemoveTagClicked);
    connect(updateTagAction, &QAction::triggered, this, &MainWindow::onUpdateTagClicked);

    // 连接帮助相关动作的信号和槽
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(documentationAction, &QAction::triggered, this, &MainWindow::showDocumentation);

    // 连接文件处理相关UI组件的信号和槽
    connect(searchButton, &QPushButton::clicked, this, [this, searchDirEdit, searchKeywordEdit]() {
        fileProcessor->searchFiles(searchDirEdit->text(), searchKeywordEdit->text());
    });

    connect(uploadButton, &QPushButton::clicked, this, [this, uploadFilePathEdit]() {
        fileProcessor->uploadFile(uploadFilePathEdit->text(), QUrl("http://yourserver.com/upload"));
    });

    connect(downloadButton, &QPushButton::clicked, this, [this, downloadUrlEdit, downloadSavePathEdit]() {
        fileProcessor->downloadFile(QUrl(downloadUrlEdit->text()), downloadSavePathEdit->text());
    });

    connect(fileProcessor, &FileProcessor::searchFinished, resultList, &QListWidget::addItems);
    connect(fileProcessor, &FileProcessor::uploadProgress, progressBar, &QProgressBar::setValue);
    connect(fileProcessor, &FileProcessor::downloadProgress, progressBar, &QProgressBar::setValue);
}

MainWindow::~MainWindow() {
    // 清理资源
    delete fileProcessor;
    delete progressBar;
}

void MainWindow::onFileActionClicked() {
    // 实现文件动作的处理逻辑，例如打开文件对话框
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*)");
    if (!filePath.isEmpty()) {
        // 处理选定的文件路径
        QMessageBox::information(this, "文件已选择", "您选择的文件是: " + filePath);
    }
}

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

        // fileList.append("删除所有文件");  // 添加 "删除所有文件" 选项

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
        // 设置一个空的 QStringListModel 以隐藏所有文件和文件夹
        QStringListModel *emptyModel = new QStringListModel(this);
        fileView->setModel(emptyModel);
        fileView->setRootIndex(QModelIndex());
        Logger::instance().log("视图已清除");
        return;
    }

    // 获取第一个文件的目录并设置为根目录
    QFileInfo firstFile(filepaths.first());
    QString directory = firstFile.absolutePath();

    // 设置 QFileSystemModel
    fileView->setModel(fileModel);
    fileModel->setRootPath(directory);
    fileView->setRootIndex(fileModel->index(directory));
    Logger::instance().log("设置根目录为：" + directory);

    // 过滤文件
    QStringList nameFilters;
    for (const QString &filePath : filepaths) {
        QFileInfo fileInfo(filePath);
        nameFilters << fileInfo.fileName();
        Logger::instance().log("添加过滤文件：" + fileInfo.fileName());
    }
    fileModel->setNameFilters(nameFilters);
    fileModel->setNameFilterDisables(false);
    fileView->setRootIndex(fileModel->index(directory));  // 设置视图的根目录

    // 设置文件名长度限制
    for (int i = 0; i < fileModel->rowCount(fileView->rootIndex()); ++i) {
        QModelIndex index = fileModel->index(i, 0, fileView->rootIndex());
        QString fileName = fileModel->fileName(index);
        if (fileName.length() > 20) {  // 限制文件名长度
            QString shortName = fileName.left(17) + "...";
            fileModel->setData(index, shortName, Qt::DisplayRole);
            Logger::instance().log("截断文件名：" + fileName + "为：" + shortName);
        }
    }
    fileView->update();  // 手动刷新视图
}

void MainWindow::onFileClicked(const QModelIndex &index) {
    QString filePath = fileModel->filePath(index);
    showFilePreview(filePath);  // 显示文件预览
}

void MainWindow::showFilePreview(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (fileInfo.isFile()) {
        if (fileInfo.suffix().toLower() == "txt" || fileInfo.suffix().toLower() == "md") {
            // 显示文本文件内容
            QFile file(filePath);
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&file);
                QString content = in.readAll();
                file.close();

                // 使用QTextEdit显示文件内容
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
            // 显示图片文件缩略图
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