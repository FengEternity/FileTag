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
    UserManager(const std::string& filename);  // 构造函数，接受文件名作为参数
    ~UserManager();  // 析构函数，保存数据到文件

    bool addUser(const std::string& username, const std::string& password, UserRole role);
    bool authenticate(const std::string& username, const std::string& password) const;
    void createUser();
    void loadUsers();  // 从文件加载用户数据
    void saveUsers() const;  // 保存用户数据到文件
    UserRole getUserRole(const std::string& username) const;

private:
    struct UserInfo {
        std::string password;
        UserRole role;
    };

    std::unordered_map<std::string, UserInfo> users;
    std::string filename;

    std::string hashPassword(const std::string& password) const;  // 密码哈希函数
};

#endif // USER_MANAGER_H
