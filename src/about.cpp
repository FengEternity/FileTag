#include "about.h"
#include "ui_about.h"

About::About(const QString &settingsFile, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::About)
        , settings(settingsFile, QSettings::IniFormat) {
    ui->setupUi(this);
    closeCheckBox = ui->closeCheckBox;
    closeCheckBox->setChecked(!settings.value("showAbout", true).toBool());

    // 手动连接信号和槽
    connect(closeCheckBox, &QCheckBox::clicked, this, &About::on_closeCheck_clicked);
    qDebug() << "About window constructed and signal-slot connected";
}

About::~About() {
    delete ui;
    settings.setValue("showAbout", !closeCheckBox->isChecked());
}

void About::on_closeCheck_clicked() {
    settings.setValue("showAbout", !closeCheckBox->isChecked());
    qDebug() << "Checkbox clicked, setting updated";
}
