#include "MultiSelectDialog.h"

MultiSelectDialog::MultiSelectDialog(const QStringList &items, QWidget *parent)
        : QDialog(parent) {
    listWidget = new QListWidget(this);
    listWidget->addItems(items);
    listWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    selectAllButton = new QPushButton(tr("Select All"), this);
    okButton = new QPushButton(tr("OK"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);

    connect(selectAllButton, &QPushButton::clicked, this, &MultiSelectDialog::selectAll);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(listWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(selectAllButton);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
    setWindowTitle(tr("选择文件"));
}

QStringList MultiSelectDialog::selectedItems() const {
    QStringList items;
    for (QListWidgetItem *item : listWidget->selectedItems()) {
        items.append(item->text());
    }
    return items;
}

void MultiSelectDialog::selectAll() {
    listWidget->selectAll();
}
