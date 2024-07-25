// FileSearch.h

#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QThreadPool>
#include <QElapsedTimer>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QVector>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include "FileSearchThread.h"

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
    void onSearchFilterChanged(const QString &text);

private:
    Ui::FileSearch *ui;
    QThreadPool *threadPool;
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
    QElapsedTimer timer;
    int updateCounter;
    int activeTaskCount;
    int totalDirectories;

    void onSearchTime(qint64 elapsedTime);
    void updateProgressLabel();
    void finishSearch();
    void stopAllTasks();

    bool isSearching;
    bool firstSearch;
    bool isStopping;
    static QVector<QString> filesBatch; // 声明静态变量
    QSet<QString> uniquePaths; // 用于记录已处理的目录
    QSet<QString> uniqueFiles; // 用于记录已记录的文件

    QVector<FileSearchThread *> activeTasks; // 新增保存活动任务的成员变量

    // 新增任务队列和同步机制的变量
    QQueue<QString> *taskQueue;
    QMutex *queueMutex;
    QWaitCondition *queueCondition;

    void enqueueDirectories(const QString &path, int depth); // 新增方法声明
};

#endif // FILESEARCH_H
