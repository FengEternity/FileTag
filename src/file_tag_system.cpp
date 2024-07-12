#include "file_tag_system.h"
#include <iostream>
#include <filesystem>

FileTagSystem::FileTagSystem(const std::string& tagsFile) : tagManager(tagsFile) {
    try {
        tagManager.loadTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void FileTagSystem::run() {
    while (true) {
        displayMenu();
        int choice;
        std::cin >> choice;
        handleChoice(choice);
    }
}

void FileTagSystem::displayMenu() const {
    std::cout << "请选择操作：\n1. 添加标签\n2. 根据标签搜索文件\n3. 删除标签\n4. 更新标签\n5. 查看所有标签\n6. 查看某个文件的标签\n7. 退出" << std::endl;
}

void FileTagSystem::handleChoice(int choice) {
    switch (choice) {
        case 1:
            addTags();
            break;
        case 2:
            searchFilesByTag();
            break;
        case 3:
            removeTag();
            break;
        case 4:
            updateTag();
            break;
        case 5:
            listAllTags();
            break;
        case 6:
            listTagsForFile();
            break;
        case 7:
            std::cout << "退出程序。" << std::endl;
            exit(0);
        default:
            std::cerr << "无效的选择。" << std::endl;
            break;
    }

    // 在每次操作后尝试保存标签
    try {
        tagManager.saveTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::string FileTagSystem::getValidInput(const std::string& prompt) const {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::cin >> input;
        if (input == "exit") {
            return "";
        } else if (!input.empty()) {
            return input;
        } else {
            std::cerr << "输入不能为空，请重新输入。" << std::endl;
        }
    }
}

std::string FileTagSystem::getTag() const {
    return getValidInput("请输入要添加的标签 (输入 'exit' 返回主界面): ");
}

std::string FileTagSystem::getValidPath() const {
    std::string path;
    while (true) {
        std::cout << "请输入文件或文件夹路径 (输入 'exit' 返回主界面): ";
        std::cin >> path;
        if (path == "exit") {
            return "";
        }
        if (std::filesystem::exists(path)) {
            return path;
        } else {
            std::cerr << "路径不存在: " << path << std::endl;
        }
    }
}

void FileTagSystem::addTags() {
    std::string path = getValidPath();
    if (path.empty()) return;

    if (std::filesystem::is_directory(path)) {
        int tagChoice;
        std::cout << "请选择操作：1. 给所有文件添加相同的标签 2. 每个文件单独添加标签" << std::endl;
        std::cin >> tagChoice;

        if (tagChoice == 1) {
            std::string tag = getTag();
            if (tag.empty()) {
                return;
            }
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    tagManager.addTag(entry.path().string(), tag);
                }
            }
        } else if (tagChoice == 2) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::cout << "文件: " << entry.path().string() << std::endl;
                    std::string tag = getTag();
                    if (tag.empty()) {
                        return;
                    }
                    tagManager.addTag(entry.path().string(), tag);
                }
            }
        } else {
            std::cerr << "无效的选择。" << std::endl;
            return;
        }
    } else {
        std::string tag = getTag();
        if (tag.empty()) {
            return;
        }
        tagManager.addTag(path, tag);
    }
}

void FileTagSystem::searchFilesByTag() {
    std::string tag = getTag();
    if (tag.empty()) return;

    std::vector<std::string> filepaths = tagManager.searchFilesByTag(tag);
    if (filepaths.empty()) {
        std::cout << "没有找到匹配的文件。" << std::endl;
    } else {
        std::cout << "匹配的文件路径：" << std::endl;
        for (const auto& filepath : filepaths) {
            std::cout << filepath << std::endl;
        }
    }
}

void FileTagSystem::removeTag() {
    std::string path = getValidPath();
    if (path.empty()) return;

    std::string tag = getTag();
    if (tag.empty()) return;

    tagManager.removeTag(path, tag);
}

void FileTagSystem::updateTag() {
    std::string path = getValidPath();
    if (path.empty()) return;

    std::string oldTag = getTag();
    if (oldTag.empty()) return;

    std::string newTag = getValidInput("请输入新的标签 (输入 'exit' 返回主界面): ");
    if (newTag.empty()) return;

    tagManager.updateTag(path, oldTag, newTag);
}

void FileTagSystem::listAllTags() const {
    auto tags = tagManager.listAllTags();
    std::cout << "所有标签：" << std::endl;
    for (const auto& tag : tags) {
        std::cout << tag << std::endl;
    }
}

void FileTagSystem::listTagsForFile() const {
    std::string path = getValidPath();
    if (path.empty()) return;

    auto tags = tagManager.listTagsForFile(path);
    std::cout << "文件 " << path << " 的标签：" << std::endl;
    for (const auto& tag : tags) {
        std::cout << tag << std::endl;
    }
}