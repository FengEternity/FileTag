#include "user_manager.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

// 构造函数，接受文件名作为参数
UserManager::UserManager(const std::string& filename) : filename(filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // 文件不存在，创建一个新的空文件
        std::ofstream newFile(filename);
        if (!newFile.is_open()) {
            throw std::runtime_error("无法创建数据文件");
        }
        newFile.close();
    } else {
        file.close();
        loadUsers();
    }
}

// 析构函数，保存数据到文件
UserManager::~UserManager() {
    saveUsers();
}

// 哈希函数，用于加密密码
std::string UserManager::hashPassword(const std::string& password) const {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

// 添加用户，返回是否成功
bool UserManager::addUser(const std::string& username, const std::string& password, UserRole role) {
    if (username.empty() || password.empty()) {
        return false; // 用户名或密码不能为空
    }
    if (users.find(username) != users.end()) {
        return false; // 用户已存在
    }
    users[username] = {hashPassword(password), role};
    return true;
}

// 验证用户，返回是否成功
bool UserManager::authenticate(const std::string& username, const std::string& password) const {
    auto it = users.find(username);
    return it != users.end() && it->second.password == hashPassword(password);
}

// 获取用户角色
UserRole UserManager::getUserRole(const std::string& username) const {
    try {
        return users.at(username).role;
    } catch (const std::out_of_range&) {
        throw std::invalid_argument("用户不存在");
    }
}

// 从文件加载用户数据
void UserManager::loadUsers() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return; // 文件不存在或无法打开
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password, roleStr;
        if (std::getline(iss, username, ',') && std::getline(iss, password, ',') && std::getline(iss, roleStr)) {
            UserRole role = (roleStr == "ADMIN") ? UserRole::ADMIN : UserRole::USER;
            users[username] = {password, role};
        }
    }
    file.close();
}

// 保存用户数据到文件
void UserManager::saveUsers() const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件进行写入");
    }

    for (const auto& pair : users) {
        file << pair.first << ',' << pair.second.password << ',' 
             << (pair.second.role == UserRole::ADMIN ? "ADMIN" : "USER") << '\n';
    }
    file.close();

    if (!file) {
        throw std::runtime_error("写入文件失败");
    } else {
        std::cout << "用户数据已保存" << std::endl;
    }
}
