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


    bool isSearching;
    bool firstSearch = true;
    static QVector<QString> filesBatch; // 声明静态变量
};

#endif // FILESEARCH_H
