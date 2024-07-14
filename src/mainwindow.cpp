#include "mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), addTagButton(new QPushButton("添加标签", this)),
          searchTagButton(new QPushButton("搜索标签", this)),
          removeTagButton(new QPushButton("删除标签", this)),
          updateTagButton(new QPushButton("更新标签", this)),
          infoLabel(new QLabel("文件标签系统", this)),
          centralWidget(new QWidget(this)), mainLayout(new QVBoxLayout),
          contentLayout(new QHBoxLayout), leftLayout(new QVBoxLayout),
          rightLayout(new QVBoxLayout), toolBar(new QToolBar(this)),
          tagListWidget(new QListWidget(this)), displayArea(new QTextEdit(this)),
          fileTagSystem("tags.csv", "users.csv") {  // 初始化 FileTagSystem 对象

    // 设置窗口标题
    setWindowTitle("文件标签系统");

    // 初始化工具栏并添加按钮
    toolBar->addAction("添加标签", this, &MainWindow::onAddTagClicked);
    toolBar->addAction("搜索标签", this, &MainWindow::onSearchTagClicked);
    toolBar->addAction("删除标签", this, &MainWindow::onRemoveTagClicked);
    toolBar->addAction("更新标签", this, &MainWindow::onUpdateTagClicked);
    addToolBar(toolBar);

    // 左侧布局：标签列表
    leftLayout->addWidget(tagListWidget);

    // 中间布局：显示区域
    displayArea->setReadOnly(true);
    rightLayout->addWidget(displayArea);

    // 内容布局
    contentLayout->addLayout(leftLayout);
    contentLayout->addLayout(rightLayout);

    // 主布局
    mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(contentLayout);

    // 设定中央窗口
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 填充标签列表
    populateTags();

    // 连接标签选择信号到槽函数
    connect(tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);
}

MainWindow::~MainWindow() {}

void MainWindow::onAddTagClicked() {
    QString filePath = QInputDialog::getText(this, "添加标签", "请输入文件路径:");
    QString tag = QInputDialog::getText(this, "添加标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        // 调用 fileTagSystem 的 addTags 函数
        fileTagSystem.addTags(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已添加", "标签已添加到文件: " + filePath);
        populateTags(); // 重新填充标签列表
    }
}

void MainWindow::onSearchTagClicked() {
    QString tag = QInputDialog::getText(this, "搜索标签", "请输入标签:");

    if (!tag.isEmpty()) {
        // 调用 fileTagSystem 的 searchFilesByTag 函数
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag.toStdString());
        QString result = "找到的文件:\n";
        for (const auto &file : files) {
            result += QString::fromStdString(file) + "\n";
        }
        displayArea->setText(result);
    }
}

void MainWindow::onRemoveTagClicked() {
    QString filePath = QInputDialog::getText(this, "删除标签", "请输入文件路径:");
    QString tag = QInputDialog::getText(this, "删除标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        // 调用 fileTagSystem 的 removeTag 函数
        fileTagSystem.removeTag(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已删除", "标签已从文件删除: " + filePath);
        populateTags(); // 重新填充标签列表
    }
}

void MainWindow::onUpdateTagClicked() {
    QString filePath = QInputDialog::getText(this, "更新标签", "请输入文件路径:");
    QString oldTag = QInputDialog::getText(this, "更新标签", "请输入旧标签:");
    QString newTag = QInputDialog::getText(this, "更新标签", "请输入新标签:");

    if (!filePath.isEmpty() && !oldTag.isEmpty() && !newTag.isEmpty()) {
        // 调用 fileTagSystem 的 updateTag 函数
        fileTagSystem.updateTag(filePath.toStdString(), oldTag.toStdString(), newTag.toStdString());
        QMessageBox::information(this, "标签已更新", "文件中的标签已更新: " + filePath);
        populateTags(); // 重新填充标签列表
    }
}

void MainWindow::onTagSelected() {
    QListWidgetItem *item = tagListWidget->currentItem();
    if (item) {
        std::string tag = item->text().toStdString();
        std::vector<std::string> files = fileTagSystem.searchFilesByTag(tag);
        QString result = "标签: " + QString::fromStdString(tag) + "\n对应的文件:\n";
        for (const auto &file : files) {
            result += QString::fromStdString(file) + "\n";
        }
        displayArea->setText(result);
    }
}

void MainWindow::populateTags() {
    tagListWidget->clear();
    std::vector<std::string> tags = fileTagSystem.listAllTags();
    for (const auto &tag : tags) {
        tagListWidget->addItem(QString::fromStdString(tag));
    }
}
