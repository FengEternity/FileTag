//
// Created by Monty-Lee  on 24-7-14.
//

#include "mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), addTagButton(new QPushButton("添加标签", this)),
          searchTagButton(new QPushButton("搜索标签", this)),
          removeTagButton(new QPushButton("删除标签", this)),
          updateTagButton(new QPushButton("更新标签", this)),
          infoLabel(new QLabel("文件标签系统", this)),
          centralWidget(new QWidget(this)), layout(new QVBoxLayout),
          fileTagSystem("tags.csv", "users.csv") {  // 初始化 FileTagSystem 对象

    // 设置窗口标题
    setWindowTitle("文件标签系统");

    // 添加控件到布局
    layout->addWidget(infoLabel);
    layout->addWidget(addTagButton);
    layout->addWidget(searchTagButton);
    layout->addWidget(removeTagButton);
    layout->addWidget(updateTagButton);

    // 连接按钮点击信号到槽函数
    connect(addTagButton, &QPushButton::clicked, this, &MainWindow::onAddTagClicked);
    connect(searchTagButton, &QPushButton::clicked, this, &MainWindow::onSearchTagClicked);
    connect(removeTagButton, &QPushButton::clicked, this, &MainWindow::onRemoveTagClicked);
    connect(updateTagButton, &QPushButton::clicked, this, &MainWindow::onUpdateTagClicked);

    // 设置中心窗口部件
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {}

void MainWindow::onAddTagClicked() {
    QString filePath = QInputDialog::getText(this, "添加标签", "请输入文件路径:");
    QString tag = QInputDialog::getText(this, "添加标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        // 调用 fileTagSystem 的 addTags 函数
        fileTagSystem.addTags(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已添加", "标签已添加到文件: " + filePath);
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
        QMessageBox::information(this, "找到的文件", result);
    }
}

void MainWindow::onRemoveTagClicked() {
    QString filePath = QInputDialog::getText(this, "删除标签", "请输入文件路径:");
    QString tag = QInputDialog::getText(this, "删除标签", "请输入标签:");

    if (!filePath.isEmpty() && !tag.isEmpty()) {
        // 调用 fileTagSystem 的 removeTag 函数
        fileTagSystem.removeTag(filePath.toStdString(), tag.toStdString());
        QMessageBox::information(this, "标签已删除", "标签已从文件删除: " + filePath);
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
    }
}

