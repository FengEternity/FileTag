#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>
#include <QSettings>
#include <QCheckBox>

namespace Ui {
    class About;
}

class About : public QWidget {
Q_OBJECT

public:
    explicit About(const QString &settingsFile, QWidget *parent = nullptr);
    ~About();

private slots:
    void on_closeCheck_clicked();

private:
    Ui::About *ui;
    QSettings settings;
    QCheckBox *closeCheckBox;
};

#endif // ABOUT_H
