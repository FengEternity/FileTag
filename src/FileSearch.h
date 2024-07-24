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
#include <QFileInfo>

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

private:
    Ui::FileSearch *ui;
    QThreadPool *threadPool;
    QPushButton *searchButton;
    QLineEdit *searchLineEdit;
    QLineEdit *pathLineEdit;
    QTableView *resultTableView;
    QStandardItemModel *tableModel;
    QPushButton *finishButton;
    QProgressBar *progressBar;
    QLabel *progressLabel;
    QElapsedTimer timer;
    int updateCounter;
    int activeTaskCount;
    int totalDirectories;

    void onSearchTime(qint64 elapsedTime);
    void updateProgressLabel();
    bool isSearching;
    bool firstSearch = true;
};

#endif // FILESEARCH_H
