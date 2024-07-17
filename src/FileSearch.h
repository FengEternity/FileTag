#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "CustomModel.h"

namespace Ui {
    class FileSearch;
}

class FileSearch : public QWidget
{
Q_OBJECT

public:
    explicit FileSearch(QWidget *parent = nullptr);
    ~FileSearch();

private slots:
    void onSearchButtonClicked();

private:
    Ui::FileSearch *ui;
    QListWidget *resultListWidget;  // 使用 QListWidget
    CustomModel *resultModel;  // Use CustomModel instead of QStandardItemModel
    QLineEdit *searchLineEdit;
    QLineEdit *pathLineEdit;
    QPushButton *searchButton;
};

#endif // FILESEARCH_H
