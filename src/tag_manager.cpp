//
// Created by Monty-Lee  on 24-7-11.
//

#include "tag_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

TagManager::TagManager(const std::string& filename) : filename(filename) {}

void TagManager::loadTags() {
    std::ifstream infile(filename);
    std::string line;

    if (!infile.is_open()) {
        std::cerr << "无法打开文件 " << filename << "，将创建一个新的文件。" << std::endl;
        std::ofstream outfile(filename); // 创建一个新的CSV文件
        if (!outfile.is_open()) {
            throw std::runtime_error("无法创建文件 " + filename);
        }
        outfile.close();
        return;
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
}

void TagManager::saveTags() const {
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

void TagManager::addTag(const std::string& filepath, const std::string& tag) {
    tags[filepath].push_back(tag);
}

std::vector<std::string> TagManager::searchFilesByTag(const std::string& tag) const {
    std::vector<std::string> filepaths;
    for (const auto& [filepath, fileTags] : tags) {
        if (std::find(fileTags.begin(), fileTags.end(), tag) != fileTags.end()) {
            filepaths.push_back(filepath);
        }
    }
    return filepaths;
}

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

std::string getTag() {
    std::string tag;
    std::cout << "请输入要添加的标签 (输入 'exit' 退出到主界面): ";
    std::cout.flush();  // 立即刷新缓冲区
    std::cin >> tag;
    return tag;
}

