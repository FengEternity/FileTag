#include "mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          infoLabel(new QLabel("文件标签系统")),
          centralWidget(new QWidget(this)), mainLayout(new QVBoxLayout),
          toolBar(new QToolBar(this)),
          tagListWidget(new QListWidget(this)), displayArea(new QTextEdit(this)),
          fileTagSystem("tags.csv", "users.csv") {  // 初始化 FileTagSystem 对象

    // 设置窗口标题
    setWindowTitle("FileTag");

    // 创建菜单栏
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // 创建文件菜单
    fileMenu = menuBar->addMenu(tr("文件"));

    // 创建编辑菜单并添加操作
    editMenu = menuBar->addMenu(tr("编辑"));
    editMenu->addAction(tr("添加标签"), this, &MainWindow::onAddTagClicked);
    editMenu->addAction(tr("搜索标签"), this, &MainWindow::onSearchTagClicked);
    editMenu->addAction(tr("删除标签"), this, &MainWindow::onRemoveTagClicked);
    editMenu->addAction(tr("更新标签"), this, &MainWindow::onUpdateTagClicked);

    // 初始化工具栏并添加按钮
    QAction *addTagAction = new QAction("添加标签", this);
    QAction *searchTagAction = new QAction("搜索标签", this);
    QAction *removeTagAction = new QAction("删除标签", this);
    QAction *updateTagAction = new QAction("更新标签", this);

    toolBar->addAction(addTagAction);
    toolBar->addAction(searchTagAction);
    toolBar->addAction(removeTagAction);
    toolBar->addAction(updateTagAction);
    addToolBar(Qt::TopToolBarArea, toolBar);

    // 使用 QSplitter 来分割左侧和右侧布局
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tagListWidget);
    splitter->addWidget(displayArea);

    // 设置左侧标签列表占20%
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    // 主布局
    // mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(splitter);

    // 设定中央窗口
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 填充标签列表
    populateTags();

    // 连接标签选择信号到槽函数
    connect(tagListWidget, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);

    // 连接按钮点击信号到槽函数
    connect(addTagAction, &QAction::triggered, this, &MainWindow::onAddTagClicked);
    connect(searchTagAction, &QAction::triggered, this, &MainWindow::onSearchTagClicked);
    connect(removeTagAction, &QAction::triggered, this, &MainWindow::onRemoveTagClicked);
    connect(updateTagAction, &QAction::triggered, this, &MainWindow::onUpdateTagClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onAddTagClicked() {
    QString filePath = QInputDialog::getText(this, "添加标签", "请输入文件路径:");
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
