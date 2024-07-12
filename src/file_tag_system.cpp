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
        if (choice == 7) break;
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
            break;
        default:
            std::cerr << "无效的选择。" << std::endl;
            break;
    }
}

void FileTagSystem::addTags() {
    std::string path = getValidPath();

    if (std::filesystem::is_directory(path)) {
        int tagChoice;
        std::cout << "请选择操作：1. 给所有文件添加相同的标签 2. 每个文件单独添加标签" << std::endl;
        std::cin >> tagChoice;

        if (tagChoice == 1) {
            std::string tag = getTag();
            if (tag == "exit") {
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
                    if (tag == "exit") {
                        try {
                            tagManager.saveTags();
                        } catch (const std::exception& e) {
                            std::cerr << e.what() << std::endl;
                            return;
                        }
                        std::cout << "已保存当前数据，返回主界面。" << std::endl;
                        break;
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
        if (tag == "exit") {
            return;
        }
        tagManager.addTag(path, tag);
    }

    try {
        tagManager.saveTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    std::cout << "标签添加成功！" << std::endl;
}

void FileTagSystem::searchFilesByTag() {
    std::string tag = getTag();
    if (tag == "exit") {
        return;
    }
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
    std::string tag = getTag();
    if (tag == "exit") {
        return;
    }
    tagManager.removeTag(path, tag);

    try {
        tagManager.saveTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    std::cout << "标签删除成功！" << std::endl;
}

void FileTagSystem::updateTag() {
    std::string path = getValidPath();
    std::string oldTag = getTag();
    if (oldTag == "exit") {
        return;
    }
    std::cout << "请输入新的标签: ";
    std::string newTag;
    std::cin >> newTag;
    tagManager.updateTag(path, oldTag, newTag);

    try {
        tagManager.saveTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    std::cout << "标签更新成功！" << std::endl;
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
    auto tags = tagManager.listTagsForFile(path);
    std::cout << "文件 " << path << " 的标签：" << std::endl;
    for (const auto& tag : tags) {
        std::cout << tag << std::endl;
    }
}
