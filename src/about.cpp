#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);

    closeCheckBox = ui->closeCheckBox;
}

About::~About()
{
    delete ui;
}


void About::on_closeCheck_clicked() {
    // 函数实现

}

