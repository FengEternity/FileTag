#ifndef FILE_TAG_SYSTEM_H
#define FILE_TAG_SYSTEM_H

#include "tag_manager.h"
#include <string>

class FileTagSystem {
public:
    explicit FileTagSystem(const std::string& tagsFile);
    void run();

private:
    void displayMenu() const;  // 显示主菜单
    void handleChoice(int choice);  // 处理用户选择
    void addTags();  // 添加标签
    void searchFilesByTag();  // 根据标签搜索文件
    void removeTag();  // 删除标签
    void updateTag();  // 更新标签
    void listAllTags() const;  // 查看所有标签
    void listTagsForFile() const;  // 查看某个文件的标签

    std::string getValidInput(const std::string& prompt) const;  // 获取有效的用户输入
    std::string getTag() const;  // 获取标签输入
    std::string getValidPath() const;  // 获取有效的路径输入

    TagManager tagManager;  // 标签管理器
};

#endif // FILE_TAG_SYSTEM_H
