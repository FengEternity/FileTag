#ifndef MULTISELECTDIALOG_H
#define MULTISELECTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>

class MultiSelectDialog : public QDialog {
Q_OBJECT

public:
    MultiSelectDialog(const QStringList &items, QWidget *parent = nullptr);
    QStringList selectedItems() const;

private slots:
    void selectAll();

private:
    QListWidget *listWidget;
    QPushButton *selectAllButton;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // MULTISELECTDIALOG_H
