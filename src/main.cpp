#include "file_tag_system.h"
#include <iostream>

int main() {
    std::cerr.rdbuf(std::cout.rdbuf());

    FileTagSystem system("tags.csv");
    system.run();

    return 0;
}
