#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <unordered_map>

enum class UserRole {
    ADMIN,
    USER
};

class UserManager {
public:
    // 添加用户，返回是否成功
    bool addUser(const std::string& username, const std::string& password, UserRole role);
    // 验证用户，返回是否成功
    bool authenticate(const std::string& username, const std::string& password) const;
    // 获取用户角色
    UserRole getUserRole(const std::string& username) const;

private:
    // 用户信息结构体
    struct UserInfo {
        std::string password;
        UserRole role;
    };
    // 用户信息存储
    std::unordered_map<std::string, UserInfo> users;
};

#endif // USER_MANAGER_H
