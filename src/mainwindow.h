#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QToolBar>
#include "file_tag_system.h"

class MainWindow : public QMainWindow {
Q_OBJECT  // 添加 Q_OBJECT 宏

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTagClicked();
    void onSearchTagClicked();
    void onRemoveTagClicked();
    void onUpdateTagClicked();
    void onTagSelected(); // 当选择标签时调用的槽函数

private:
    QPushButton *addTagButton;
    QPushButton *searchTagButton;
    QPushButton *removeTagButton;
    QPushButton *updateTagButton;
    QLabel *infoLabel;

    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *contentLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;

    QToolBar *toolBar;
    QListWidget *tagListWidget;
    QTextEdit *displayArea;

    FileTagSystem fileTagSystem;  // 添加 FileTagSystem 成员

    void populateTags();  // 填充标签列表
};

#endif // MAINWINDOW_H
