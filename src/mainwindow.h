#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include "file_tag_system.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void mainWindowClosed();

private slots:
    void onAddTagClicked();
    void onSearchTagClicked();
    void onRemoveTagClicked();
    void onUpdateTagClicked();
    void onFileActionClicked();
    void onTagSelected();
    void onFileClicked(const QModelIndex &index);
    void showAboutDialog();
    void showDocumentation();
    void initializeView();
    void onFileSearchClicked();
    void onFileTransferClicked();

    void on_actionHome_triggered();

private:
    Ui::MainWindow *ui;

    QFileSystemModel *fileModel;
    FileTagSystem fileTagSystem;

    void populateTags();
    void displayFiles(const QStringList& filepaths);
    void showFilePreview(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
