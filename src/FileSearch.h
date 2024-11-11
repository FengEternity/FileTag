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
#include <QCheckBox>

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
    void updateProgressLabel(int value, int total);

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
    QCheckBox *includeSystemFilesCheckBox; // 声明复选框
    QLabel *progressLabel;

    FileSearchCore *searchCore;

};

#endif // FILESEARCH_H
