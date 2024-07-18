#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "CustomModel.h"
#include "FileSearchThread.h" // 确保包含头文件

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
    void onFileFound(const QString &filePath);
    void onSearchFinished();
    void onFinishButtonClicked();  // 新增的槽函数
    void onSearchTime(qint64 elapsedTime); // 新增的槽函数

private:
    Ui::FileSearch *ui;
    CustomModel *resultModel;
    QPushButton *searchButton;
    QLineEdit *searchLineEdit;
    QLineEdit *pathLineEdit;
    QListWidget *resultListWidget;
    QPushButton *finishButton;      // 新增
    FileSearchThread *currentSearchThread; // 新增
    QElapsedTimer timer; // 新增
};

#endif // FILESEARCH_H
