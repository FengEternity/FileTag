# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(FileTag)

set(CMAKE_CXX_STANDARD 11)

add_executable(FileTag main.cpp)

# 添加自定义清理目标
add_custom_target(clean-all
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_clean.cmake
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
)

# 创建 cmake_clean.cmake 文件
file(WRITE ${CMAKE_BINARY_DIR}/cmake_clean.cmake "file(REMOVE_RECURSE ${CMAKE_BINARY_DIR})")
