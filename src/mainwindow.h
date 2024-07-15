#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QMenuBar>
#include "file_tag_system.h"

class MainWindow : public QMainWindow {
Q_OBJECT

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
    QLabel *infoLabel;

    QWidget *centralWidget;
    QHBoxLayout *mainLayout; // 修改为QHBoxLayout
    QVBoxLayout *sideBarLayout;
    QVBoxLayout *contentLayout;

    QToolBar *toolBar;

    QMenuBar *menuBar;
    QMenu *fileMenu;
    QMenu *editMenu;

    QListWidget *tagListWidget;
    QTextEdit *displayArea;

    FileTagSystem fileTagSystem;  // 添加 FileTagSystem 成员

    void populateTags();  // 填充标签列表
};

#endif // MAINWINDOW_H
