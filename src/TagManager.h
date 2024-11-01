#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

class TagManager {
public:
    TagManager(const std::string& filename);  // 构造函数
    void loadTags();  // 加载标签
    void saveTags() const;  // 保存标签
    void addTag(const std::string& filepath, const std::string& tag);  // 添加标签
    void removeTag(const std::string& filepath, const std::string& tag);  // 删除标签
    void updateTag(const std::string& filepath, const std::string& oldTag, const std::string& newTag);  // 更新标签
    std::vector<std::string> searchFilesByTag(const std::string& tag) const;  // 根据标签搜索文件
    std::vector<std::string> listAllTags() const;  // 查看所有标签
    std::vector<std::string> listTagsForFile(const std::string& filepath) const;  // 查看某个文件的标签

private:
    std::unordered_map<std::string, std::vector<std::string>> tags;  // 标签数据
    std::string filename;  // 标签文件名
};

std::string getValidPath();  // 获取有效的路径
std::string getTag();  // 获取标签

#endif // TAG_MANAGER_H
