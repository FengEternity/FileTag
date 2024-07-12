cmake_minimum_required(VERSION 3.10)
project(FileTag)

set(CMAKE_CXX_STANDARD 17)

# 将 main.cpp、file_tag_system.cpp 和 tag_manager.cpp 编译为可执行文件 FileTag
add_executable(FileTag 
        src/main.cpp 
        src/file_tag_system.cpp 
        src/tag_manager.cpp 
        src/file_tag_system.h 
        src/tag_manager.h
        src/user_manager.h
        src/user_manager.cpp
        )

# 添加自定义目标 clean-all，用于清理生成的文件
add_custom_target(clean-all
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_clean.cmake
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
)

# 创建 cmake_clean.cmake 文件，用于清理生成目录
file(WRITE ${CMAKE_BINARY_DIR}/cmake_clean.cmake "file(REMOVE_RECURSE ${CMAKE_BINARY_DIR})")
