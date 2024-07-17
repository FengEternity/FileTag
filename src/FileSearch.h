// FileSearch.h
#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <QWidget>

namespace Ui {
    class FileSearch;
}

class FileSearch : public QWidget
{
Q_OBJECT

public:
    explicit FileSearch(QWidget *parent = nullptr);
    ~FileSearch();

private:
    Ui::FileSearch *ui;
};

#endif // FILESEARCH_H
