#include "tag_manager.h"
#include <iostream>
#include <filesystem>

int main() {
    // 将 std::cerr 重定向到 std::cout
    std::cerr.rdbuf(std::cout.rdbuf());

    const std::string tagsFile = "tags.csv";
    std::cout << "标签文件路径: " << tagsFile << std::endl;
    std::cout.flush();  // 立即刷新缓冲区

    TagManager tagManager(tagsFile);
    try {
        tagManager.loadTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    while (true) {
        int choice;
        std::cout << "请选择操作：1. 添加标签 2. 根据标签搜索文件 3. 退出" << std::endl;
        std::cin >> choice;

        if (choice == 1) {
            std::string path = getValidPath();

            if (std::filesystem::is_directory(path)) {
                int tagChoice;
                std::cout << "请选择操作：1. 给所有文件添加相同的标签 2. 每个文件单独添加标签" << std::endl;
                std::cin >> tagChoice;

                if (tagChoice == 1) {
                    std::string tag = getTag();
                    if (tag == "exit") {
                        continue;
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
                                    return 1;
                                }
                                std::cout << "已保存当前数据，返回主界面。" << std::endl;
                                break;
                            }
                            tagManager.addTag(entry.path().string(), tag);
                        }
                    }
                } else {
                    std::cerr << "无效的选择。" << std::endl;
                    continue;
                }
            } else {
                std::string tag = getTag();
                if (tag == "exit") {
                    continue;
                }
                tagManager.addTag(path, tag);
            }

            try {
                tagManager.saveTags();
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return 1;
            }
            std::cout << "标签添加成功！" << std::endl;
        } else if (choice == 2) {
            std::string tag = getTag();
            if (tag == "exit") {
                continue;
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
        } else if (choice == 3) {
            std::cout << "退出程序。" << std::endl;
            break;
        } else {
            std::cerr << "无效的选择。" << std::endl;
        }
    }

    return 0;
}
