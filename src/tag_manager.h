#ifndef TAG_MANAGER_H
#define TAG_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

class TagManager {
public:
    TagManager(const std::string& filename);
    void loadTags();
    void saveTags() const;
    void addTag(const std::string& filepath, const std::string& tag);
    void removeTag(const std::string& filepath, const std::string& tag);
    void updateTag(const std::string& filepath, const std::string& oldTag, const std::string& newTag);
    std::vector<std::string> searchFilesByTag(const std::string& tag) const;
    std::vector<std::string> listAllTags() const;
    std::vector<std::string> listTagsForFile(const std::string& filepath) const;

private:
    std::unordered_map<std::string, std::vector<std::string>> tags;
    std::string filename;
};

std::string getValidPath();
std::string getTag();

#endif // TAG_MANAGER_H
