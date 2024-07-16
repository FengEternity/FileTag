#include "mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
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


MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          centralWidget(new QWidget(this)),
          mainLayout(new QHBoxLayout),
          sideBarLayout(new QVBoxLayout),
          contentLayout(new QVBoxLayout),
          tagListWidget(new QListWidget(this)),
          fileView(new QListView(this)),  // 初始化 fileView
          fileModel(new QFileSystemModel(this)),  // 初始化 fileModel
          fileTagSystem("tags.csv", "users.csv") {

    setWindowTitle("FileTag");
    resize(600,400);

    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    fileMenu = menuBar->addMenu(tr("文件"));
    tagMenu = menuBar->addMenu(tr("标签"));
    helpMenu = menuBar->addMenu(tr("帮助"));

    tagMenu->addAction(tr("添加标签"), this, &MainWindow::onAddTagClicked);
    tagMenu->addAction(tr("搜索标签"), this, &MainWindow::onSearchTagClicked);
    tagMenu->addAction(tr("删除标签"), this, &MainWindow::onRemoveTagClicked);
    tagMenu->addAction(tr("更新标签"), this, &MainWindow::onUpdateTagClicked);

    QAction *addTagAction = new QAction("添加标签", this);
    QAction *searchTagAction = new QAction("搜索标签", this);
    QAction *removeTagAction = new QAction("删除标签", this);
    QAction *updateTagAction = new QAction("更新标签", this);

    toolBar = new QToolBar(this);
    QToolButton *toolButton = new QToolButton(this);
    toolButton->setText("标签");
    toolButton->setPopupMode(QToolButton::InstantPopup);

    QMenu *tagMenu = new QMenu(toolButton);
    tagMenu->addAction(addTagAction);
    tagMenu->addAction(searchTagAction);
    tagMenu->addAction(removeTagAction);
    tagMenu->addAction(updateTagAction);
    toolButton->setMenu(tagMenu);



    toolBar->addWidget(toolButton);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tagListWidget);
    splitter->addWidget(fileView);  // 添加 fileView

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    mainLayout->addWidget(splitter);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    QFile file(":/stylesheet.qss"); // 假设样式表文件名为 stylesheet.qss
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QTextStream(&file).readAll();
        this->setStyleSheet(styleSheet);
        file.close();
    }

    fileModel->setRootPath(QDir::currentPath());
    fileView->setModel(fileModel);
    fileView->setRootIndex(fileModel->index(QDir::currentPath()));
    fileView->setViewMode(QListView::IconMode);  // 设置为图标模式
    fileView->setIconSize(QSize(64, 64));  // 设置图标大小

    populateTags();

    connect(tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);
    connect(fileView, &QListView::clicked, this, &MainWindow::onFileClicked);  // 连接文件点击信号

    connect(addTagAction, &QAction::triggered, this, &MainWindow::onAddTagClicked);
    connect(searchTagAction, &QAction::triggered, this, &MainWindow::onSearchTagClicked);
    connect(removeTagAction, &QAction::triggered, this, &MainWindow::onRemoveTagClicked);
    connect(updateTagAction, &QAction::triggered, this, &MainWindow::onUpdateTagClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onAddTagClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*)");
    if (filePath.isEmpty()) {
        return;
    }
    QString tag = QInputDialog::getText(this, "添加标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        fileTagSystem.addTags(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已添加", "标签已添加到文件: " + filePath);
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
    QString filePath = QInputDialog::getText(this, "删除标签", "请输入文件路径:");
    QString tag = QInputDialog::getText(this, "删除标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        fileTagSystem.removeTag(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已删除", "标签已从文件删除: " + filePath);
        populateTags();
    }
}

void MainWindow::onUpdateTagClicked() {
    QString filePath = QInputDialog::getText(this, "更新标签", "请输入文件路径:");
    QString oldTag = QInputDialog::getText(this, "更新标签", "请输入旧标签:");
    QString newTag = QInputDialog::getText(this, "更新标签", "请输入新标签:");

    if (!filePath.isEmpty() && !oldTag.isEmpty() && !newTag.isEmpty()) {
        fileTagSystem.updateTag(filePath.toStdString(), oldTag.toStdString(), newTag.toStdString());
        QMessageBox::information(this, "标签已更新", "文件中的标签已更新: " + filePath);
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

void MainWindow::displayFiles(const QStringList& filepaths) {
    qDebug() << "displayFiles 调用参数：" << filepaths;

    if (filepaths.isEmpty()) {
        fileView->setRootIndex(QModelIndex());
        fileModel->setNameFilters(QStringList());  // 清除过滤器
        fileView->update();  // 手动刷新视图
        qDebug() << "视图已清除";
        return;
    }

    // 清除视图缓存
    fileModel->setNameFilters(QStringList());  // 清除任何现有的文件名过滤器
    fileModel->setRootPath("");  // 重置根路径
    fileView->setRootIndex(QModelIndex());  // 清除视图的根目录

    // 获取第一个文件的目录并设置为根目录
    QFileInfo firstFile(filepaths.first());
    QString directory = firstFile.absolutePath();
    fileView->setRootIndex(fileModel->index(directory));
    fileView->update();  // 手动刷新视图
    qDebug() << "设置根目录为：" << directory;

    // 过滤文件
    QStringList nameFilters;
    for (const QString &filePath : filepaths) {
        QFileInfo fileInfo(filePath);
        nameFilters << fileInfo.fileName();
        qDebug() << "添加过滤文件：" << fileInfo.fileName();
    }
    fileModel->setNameFilters(nameFilters);
    fileModel->setNameFilterDisables(false);
    fileModel->setRootPath(directory);  // 设置根路径
    fileView->update();  // 手动刷新视图
    qDebug() << "设置文件名过滤器";

    // 设置文件名长度限制
    for (int i = 0; i < fileModel->rowCount(fileView->rootIndex()); ++i) {
        QModelIndex index = fileModel->index(i, 0, fileView->rootIndex());
        QString fileName = fileModel->fileName(index);
        if (fileName.length() > 20) {  // 限制文件名长度
            QString shortName = fileName.left(17) + "...";
            fileModel->setData(index, shortName, Qt::DisplayRole);
            qDebug() << "截断文件名：" << fileName << "为：" << shortName;
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
