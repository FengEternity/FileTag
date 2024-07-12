#include "user_manager.h"

// 添加用户，返回是否成功
bool UserManager::addUser(const std::string& username, const std::string& password, UserRole role) {
    if (users.find(username) != users.end()) {
        return false; // 用户已存在
    }
    users[username] = {password, role};
    return true;
}

// 验证用户，返回是否成功
bool UserManager::authenticate(const std::string& username, const std::string& password) const {
    auto it = users.find(username);
    if (it != users.end() && it->second.password == password) {
        return true;
    }
    return false;
}

// 获取用户角色
UserRole UserManager::getUserRole(const std::string& username) const {
    return users.at(username).role;
}
