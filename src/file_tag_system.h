#ifndef FILE_TAG_SYSTEM_H
#define FILE_TAG_SYSTEM_H

#include "tag_manager.h"
#include <string>

class FileTagSystem {
public:
    // 构造函数，接受标签文件路径作为参数
    explicit FileTagSystem(const std::string& tagsFile);
    // 主运行函数，控制程序的主循环
    void run();

private:
    // 显示主菜单
    void displayMenu() const;
    // 处理用户的菜单选择
    void handleChoice(int choice);
    // 添加标签的函数
    void addTags();
    // 根据标签搜索文件的函数
    void searchFilesByTag();
    // 删除标签的函数
    void removeTag();
    // 更新标签的函数
    void updateTag();
    // 列出所有标签的函数
    void listAllTags() const;
    // 列出某个文件的所有标签的函数
    void listTagsForFile() const;

    // 获取有效用户输入的函数，带有提示信息
    std::string getValidInput(const std::string& prompt) const;
    // 获取标签输入的函数
    std::string getTag() const;
    // 获取有效路径输入的函数
    std::string getValidPath() const;

    // 标签管理器对象
    TagManager tagManager;
};

#endif // FILE_TAG_SYSTEM_H
