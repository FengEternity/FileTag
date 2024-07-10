#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <filesystem>
#include <stdexcept>

// 从CSV文件加载标签
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

// 保存标签到CSV文件
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

// 添加标签到文件
void addTag(std::unordered_map<std::string, std::vector<std::string>>& tags, const std::string& filepath, const std::string& tag) {
    tags[filepath].push_back(tag);
}

// 获取有效的文件路径
std::string getValidFilePath() {
    std::string filepath;
    while (true) {
        std::cout << "请输入文件路径: ";
        std::cin >> filepath;

        // 检查文件路径是否存在
        if (std::__fs::filesystem::exists(filepath)) {
            break;
        } else {
            std::cerr << "文件路径不存在: " << filepath << std::endl;
        }
    }
    return filepath;
}

// 获取标签
std::string getTag() {
    std::string tag;
    std::cout << "请输入要添加的标签: ";
    std::cin >> tag;
    return tag;
}

int main() {
    const std::string tagsFile = "tags.csv";
    std::cout << "标签文件路径: " << tagsFile << std::endl;

    std::unordered_map<std::string, std::vector<std::string>> tags;
    try {
        tags = loadTags(tagsFile);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::string filepath = getValidFilePath();
    std::string tag = getTag();

    addTag(tags, filepath, tag);

    try {
        saveTags(tags, tagsFile);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << "标签添加成功！" << std::endl;
    return 0;
}
