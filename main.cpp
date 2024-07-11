#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <filesystem>
#include <stdexcept>

// 加载标签
std::unordered_map<std::string, std::vector<std::string>> loadTags(const std::string& filename) {
    std::unordered_map<std::string, std::vector<std::string>> tags;
    std::ifstream infile(filename);
    std::string line;

    if (!infile.is_open()) {
        std::cerr << "无法打开文件 " << filename << "，将创建一个新的文件。" << std::endl;
        std::ofstream outfile(filename); // 创建一个新的CSV文件
        if (!outfile.is_open()) {
            throw std::runtime_error("无法创建文件 " + filename);
        }
        outfile.close();
        return tags;
    }

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string filepath, tag;
        if (std::getline(iss, filepath, ',')) {
            std::vector<std::string> fileTags;
            while (std::getline(iss, tag, ',')) {
                fileTags.push_back(tag);
            }
            tags[filepath] = fileTags;
        }
    }
    return tags;
}

// 保存标签
void saveTags(const std::unordered_map<std::string, std::vector<std::string>>& tags, const std::string& filename) {
    std::ofstream outfile(filename);

    if (!outfile.is_open()) {
        throw std::runtime_error("无法打开文件 " + filename);
    }

    for (const auto& [filepath, fileTags] : tags) {
        outfile << filepath;
        for (const auto& tag : fileTags) {
            outfile << "," << tag;
        }
        outfile << std::endl;
    }
}

// 添加标签
void addTag(std::unordered_map<std::string, std::vector<std::string>>& tags, const std::string& filepath, const std::string& tag) {
    tags[filepath].push_back(tag);
}

// 获取有效的文件或文件夹路径
std::string getValidPath() {
    std::string path;
    while (true) {
        std::cout << "请输入文件或文件夹路径: ";
        std::cout.flush();  // 立即刷新缓冲区
        std::cin >> path;

        // 检查路径是否存在
        if (std::filesystem::exists(path)) {
            break;
        } else {
            std::cerr << "路径不存在: " << path << std::endl;
            std::cout.flush(); // 立即刷新缓冲区
        }
    }
    return path;
}

// 获取标签
std::string getTag() {
    std::string tag;
    std::cout << "请输入要添加的标签: ";
    std::cout.flush();  // 立即刷新缓冲区
    std::cin >> tag;
    return tag;
}

// 根据标签搜索文件
std::vector<std::string> searchFilesByTag(const std::unordered_map<std::string, std::vector<std::string>>& tags, const std::string& tag) {
    std::vector<std::string> filepaths;
    for (const auto& [filepath, fileTags] : tags) {
        if (std::find(fileTags.begin(), fileTags.end(), tag) != fileTags.end()) {
            filepaths.push_back(filepath);
        }
    }
    return filepaths;
}

int main() {
    // 将 std::cerr 重定向到 std::cout
    std::cerr.rdbuf(std::cout.rdbuf());

    const std::string tagsFile = "tags.csv";
    std::cout << "标签文件路径: " << tagsFile << std::endl;
    std::cout.flush();  // 立即刷新缓冲区

    std::unordered_map<std::string, std::vector<std::string>> tags;
    try {
        tags = loadTags(tagsFile);
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
                    for (const auto& entry : std::filesystem::directory_iterator(path)) {
                        if (entry.is_regular_file()) {
                            addTag(tags, entry.path().string(), tag);
                        }
                    }
                } else if (tagChoice == 2) {
                    for (const auto& entry : std::filesystem::directory_iterator(path)) {
                        if (entry.is_regular_file()) {
                            std::cout << "文件: " << entry.path().string() << std::endl;
                            std::string tag = getTag();
                            addTag(tags, entry.path().string(), tag);
                        }
                    }
                } else {
                    std::cerr << "无效的选择。" << std::endl;
                    continue;
                }
            } else {
                std::string tag = getTag();
                addTag(tags, path, tag);
            }

            try {
                saveTags(tags, tagsFile);
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return 1;
            }
            std::cout << "标签添加成功！" << std::endl;
        } else if (choice == 2) {
            std::string tag = getTag();
            std::vector<std::string> filepaths = searchFilesByTag(tags, tag);
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
