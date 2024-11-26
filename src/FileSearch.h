/*
 * FileSearch.h
 * Author: Montee
 * CreateDate: 2024-11-1
 * Updater: Montee
 * UpdateDate: 2024-11-1
 * Summary: 文件搜索窗口类的实现文件
 */

#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QCheckBox> // 添加 QCheckBox 头文件

#include "FileSearchCore.h"

namespace Ui {
    class FileSearch;
}

class FileSearch : public QWidget {
Q_OBJECT

public:
    explicit FileSearch(QWidget *parent = nullptr);
    ~FileSearch();

private slots:
    void onSearchButtonClicked();
    void onFinishButtonClicked();
    void onSearchFilterChanged(const QString &text);
    void onFileFound(const QString &filePath);
    void onSearchFinished();
    void updateProgress(int value, int total);

private:
    Ui::FileSearch *ui;
    QPushButton *searchButton;
    QLineEdit *searchLineEdit;
    QLineEdit *pathLineEdit;
    QLineEdit *filterLineEdit;
    QTableView *resultTableView;
    QStandardItemModel *tableModel;
    QSortFilterProxyModel *proxyModel;
    QPushButton *finishButton;
    QProgressBar *progressBar;
    QLabel *progressLabel;
    QCheckBox *systemFilesCheckBox; // 新增复选框指针


    FileSearchCore *searchCore;

    void updateProgressLabel(int value, int total);
};

#endif // FILESEARCH_H
