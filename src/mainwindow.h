#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListView>
#include <QFileSystemModel>
#include <QToolBar>
#include <QMenuBar>
#include <QFile>
#include <QTextStream>
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
    void onTagSelected();
    void onFileClicked(const QModelIndex &index);  // 文件点击槽函数
    void showAboutDialog();
    void showDocumentation();

private:
    QLabel *infoLabel;

    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QVBoxLayout *sideBarLayout;
    QVBoxLayout *contentLayout;

    QToolBar *toolBar;

    QMenuBar *menuBar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *tagMenu;
    QMenu *helpMenu;

    QListWidget *tagListWidget;
    QListView *fileView;  // 使用 QListView
    QFileSystemModel *fileModel;  // 使用 QFileSystemModel

    FileTagSystem fileTagSystem;  // 添加 FileTagSystem 成员

    void populateTags();  // 填充标签列表
    void displayFiles(const QStringList& filepaths);  // 显示文件列表
    void showFilePreview(const QString &filePath);  // 显示文件预览

};

#endif // MAINWINDOW_H
