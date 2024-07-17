// FileSearch.cpp
#include "FileSearch.h"
#include "ui_FileSearch.h"

FileSearch::FileSearch(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::FileSearch)
{
    ui->setupUi(this);
}

FileSearch::~FileSearch()
{
    delete ui;
}
