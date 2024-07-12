#ifndef FILE_TAG_SYSTEM_H
#define FILE_TAG_SYSTEM_H

#include "tag_manager.h"

class FileTagSystem {
public:
    explicit FileTagSystem(const std::string& tagsFile);
    void run();

private:
    void displayMenu() const;
    void handleChoice(int choice);
    void addTags();
    void searchFilesByTag();
    void removeTag();
    void updateTag();
    void listAllTags() const;
    void listTagsForFile() const;

    TagManager tagManager;
};

#endif // FILE_TAG_SYSTEM_H
