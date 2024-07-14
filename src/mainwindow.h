#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "file_tag_system.h"

class MainWindow : public QMainWindow {
Q_OBJECT  // 添加 Q_OBJECT 宏

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTagClicked();
    void onSearchTagClicked();
    void onRemoveTagClicked();
    void onUpdateTagClicked();

private:
    QPushButton *addTagButton;
    QPushButton *searchTagButton;
    QPushButton *removeTagButton;
    QPushButton *updateTagButton;
    QLabel *infoLabel;

    QWidget *centralWidget;
    QVBoxLayout *layout;

    FileTagSystem fileTagSystem;  // 添加 FileTagSystem 成员
};

#endif // MAINWINDOW_H
