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
    bool firstSearch = true;
    bool isStopping = false;
    static QVector<QString> filesBatch; // 声明静态变量

    QVector<FileSearchThread *> activeTasks; // 新增保存活动任务的成员变量
};

#endif // FILESEARCH_H
