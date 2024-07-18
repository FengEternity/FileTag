#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "CustomModel.h"
#include "FileSearchThread.h"
#include <QElapsedTimer> // 确保包含 QElapsedTimer
#include <QVector>

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
    void onFinishButtonClicked();
    void onSearchTime(qint64 elapsedTime);

private:
    Ui::FileSearch *ui;
    CustomModel *resultModel;
    QPushButton *searchButton;
    QLineEdit *searchLineEdit;
    QLineEdit *pathLineEdit;
    QListWidget *resultListWidget;
    QPushButton *finishButton;
    QVector<FileSearchThread*> searchThreads; // 新增：用于存储多个搜索线程
    QElapsedTimer timer; // 新增：用于手动计算时间
};

#endif // FILESEARCH_H