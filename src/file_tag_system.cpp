#include "file_tag_system.h"
#include "user_manager.h"
#include <iostream>
#include <filesystem>

// 构造函数，初始化 FileTagSystem 对象
FileTagSystem::FileTagSystem(const std::string& tagsFile, const std::string& usersFile)
    : tagManager(tagsFile), userManager(usersFile) {
    // 初始化用户管理器，添加一些默认用户
    userManager.addUser("admin", "admin123", UserRole::ADMIN);
    userManager.addUser("user", "user123", UserRole::USER);

    try {
        tagManager.loadTags(); // 尝试加载标签数据
        userManager.loadUsers(); // 添加这行确保加载用户数据
    } catch (const std::exception& e) {
        // 如果加载失败，输出错误信息并退出程序
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

// 运行系统的主循环
void FileTagSystem::run() {
    // 用户登录
    if (!login()) {
        std::cerr << "登录失败," ;
        std::cout << "是否注册新用户？(y/n): ";
        char choice;
        std::cin >> choice;
        if (choice == 'y') {
            std::string username, password;
            int role;
            std::cout << "请输入新用户名: ";
            std::cin >> username;
            std::cout << "请输入密码: ";
            std::cin >> password;
            std::cout << "请选择角色 (0 - 管理员, 1 - 普通用户): ";
            std::cin >> role;
            if (userManager.addUser(username, password, static_cast<UserRole>(role))) {
                std::cout << "用户添加成功。" << std::endl;
            } else {
                std::cerr << "用户已存在。" << std::endl;
            }
        } else {
            std::cerr << "退出程序。" << std::endl;
            exit(1);
        }
        // return;
    }

    // 根据用户角色显示不同的菜单并处理选择
    while (true) {
        if (currentUserRole == UserRole::ADMIN) {
            displayAdminMenu();
            int choice;
            std::cin >> choice;
            handleAdminChoice(choice);
        } else {
            displayMenu();
            int choice;
            std::cin >> choice;
            handleChoice(choice);
        }
    }
}

// 用户登录函数
bool FileTagSystem::login() {
    std::string username, password;
    std::cout << "请输入用户名: ";
    std::cin >> username;
    std::cout << "请输入密码: ";
    std::cin >> password;

    // 验证用户
    if (userManager.authenticate(username, password)) {
        currentUser = username;
        currentUserRole = userManager.getUserRole(username);
        return true;
    }
    return false;
}

// 显示管理员菜单
void FileTagSystem::displayAdminMenu() const {
    std::cout << "管理员菜单：\n1. 添加标签\n2. 根据标签搜索文件\n3. 删除标签\n4. 更新标签\n5. 查看所有标签\n6. 查看某个文件的标签\n7. 添加用户\n8. 退出" << std::endl;
}

// 处理管理员的选择
void FileTagSystem::handleAdminChoice(int choice) {
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
        case 7: {
            std::string username, password;
            int role;
            std::cout << "请输入新用户名: ";
            std::cin >> username;
            std::cout << "请输入密码: ";
            std::cin >> password;
            std::cout << "请选择角色 (0 - 管理员, 1 - 普通用户): ";
            std::cin >> role;
            if (userManager.addUser(username, password, static_cast<UserRole>(role))) {
                std::cout << "用户添加成功。" << std::endl;
            } else {
                std::cerr << "用户已存在。" << std::endl;
            }
            break;
        }
        case 8:
            std::cout << "退出程序。" << std::endl;
            exit(0);
        default:
            std::cerr << "无效的选择。" << std::endl;
            break;
    }

    // 保存标签数据
    try {
        tagManager.saveTags();
        userManager.saveUsers();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

// 显示普通用户菜单
void FileTagSystem::displayMenu() const {
    std::cout << "用户菜单：\n1. 添加标签\n2. 根据标签搜索文件\n3. 删除标签\n4. 更新标签\n5. 查看所有标签\n6. 查看某个文件的标签\n7. 退出" << std::endl;
}

// 处理普通用户的选择
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

    // 保存标签数据
    try {
        userManager.saveUsers();
        tagManager.saveTags();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

// 获取有效用户输入的函数，带有提示信息
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

// 获取标签输入的函数
std::string FileTagSystem::getTag() const {
    return getValidInput("请输入要添加的标签 (输入 'exit' 返回主界面): ");
}

// 获取有效路径输入的函数
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

// 添加标签的函数
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
                    auto existingTags = tagManager.listTagsForFile(entry.path().string());
                    if (!existingTags.empty()) {
                        std::cout << "文件 " << entry.path().string() << " 已有标签: ";
                        for (const auto& t : existingTags) {
                            std::cout << t << " ";
                        }
                        std::cout << "\n请选择操作：1. 修改已有标签 2. 新增标签 3. 不做任何修改" << std::endl;
                        int choice;
                        std::cin >> choice;
                        if (choice == 1) {
                            tagManager.updateTag(entry.path().string(), existingTags[0], tag); // 简单起见，假设只修改第一个标签
                        } else if (choice == 2) {
                            tagManager.addTag(entry.path().string(), tag);
                        }
                    } else {
                        tagManager.addTag(entry.path().string(), tag);
                    }
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
                    auto existingTags = tagManager.listTagsForFile(entry.path().string());
                    if (!existingTags.empty()) {
                        std::cout << "文件 " << entry.path().string() << " 已有标签: ";
                        for (const auto& t : existingTags) {
                            std::cout << t << " ";
                        }
                        std::cout << "\n请选择操作：1. 修改已有标签 2. 新增标签 3. 不做任何修改" << std::endl;
                        int choice;
                        std::cin >> choice;
                        if (choice == 1) {
                            tagManager.updateTag(entry.path().string(), existingTags[0], tag); // 简单起见，假设只修改第一个标签
                        } else if (choice == 2) {
                            tagManager.addTag(entry.path().string(), tag);
                        }
                    } else {
                        tagManager.addTag(entry.path().string(), tag);
                    }
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
        auto existingTags = tagManager.listTagsForFile(path);
        if (!existingTags.empty()) {
            std::cout << "文件 " << path << " 已有标签: ";
            for (const auto& t : existingTags) {
                std::cout << t << " ";
            }
            std::cout << "\n请选择操作：1. 修改已有标签 2. 新增标签 3. 不做任何修改" << std::endl;
            int choice;
            std::cin >> choice;
            if (choice == 1) {
                tagManager.updateTag(path, existingTags[0], tag); // 简单起见，假设只修改第一个标签
            } else if (choice == 2) {
                tagManager.addTag(path, tag);
            }
        } else {
            tagManager.addTag(path, tag);
        }
    }
}

// 根据标签搜索文件的函数
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

// 删除标签的函数
void FileTagSystem::removeTag() {
    std::string path = getValidPath();
    if (path.empty()) return;

    std::string tag = getTag();
    if (tag.empty()) return;

    tagManager.removeTag(path, tag);
}

// 更新标签的函数
void FileTagSystem::updateTag() {
    std::string path = getValidPath();
    if (path.empty()) return;

    std::string oldTag = getTag();
    if (oldTag.empty()) return;

    std::string newTag = getValidInput("请输入新的标签 (输入 'exit' 返回主界面): ");
    if (newTag.empty()) return;

    tagManager.updateTag(path, oldTag, newTag);
}

// 列出所有标签的函数
void FileTagSystem::listAllTags() const {
    auto tags = tagManager.listAllTags();
    std::cout << "所有标签：" << std::endl;
    for (const auto& tag : tags) {
        std::cout << tag << std::endl;
    }
}

// 列出某个文件的所有标签的函数
void FileTagSystem::listTagsForFile() const {
    std::string path = getValidPath();
    if (path.empty()) return;

    auto tags = tagManager.listTagsForFile(path);
    std::cout << "文件 " << path << " 的标签：" << std::endl;
    for (const auto& tag : tags) {
        std::cout << tag << std::endl;
    }
}
