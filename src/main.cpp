#include "file_tag_system.h"
#include <iostream>

int main() {
    // 将 std::cerr 重定向到 std::cout
    std::cerr.rdbuf(std::cout.rdbuf());

    // 初始化 FileTagSystem，并运行程序
    FileTagSystem system("tags.csv", "users.csv");
    system.run();

    return 0;
}
