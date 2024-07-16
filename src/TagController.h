#ifndef TAGCONTROLLER_H
#define TAGCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "tag_manager.h"
#include "file_tag_system.h"

class TagController : public QObject {
Q_OBJECT

public:
    explicit TagController(const std::string &filename, FileTagSystem &fileTagSystem, QObject *parent = nullptr);

signals:
    void tagsUpdated();
    void displayFiles(const QStringList &filePaths);

public slots:
    void addTag(QWidget *parentWidget);
    void searchTag(QWidget *parentWidget);
    void removeTag(QWidget *parentWidget);
    void updateTag(QWidget *parentWidget);

private:
    TagManager tagManager;
    FileTagSystem &fileTagSystem;
};

#endif // TAGCONTROLLER_H
